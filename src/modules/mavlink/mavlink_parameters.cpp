#include "mavlink_parameters.h"
#include "mavlink_main.h"
#include "mavlink_bridge_header.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace
{
constexpr uint8_t PT_UINT8  = 1;
constexpr uint8_t PT_UINT32 = 5;
constexpr uint8_t PT_INT32  = 6;
constexpr uint8_t PT_REAL32 = 9;
constexpr uint8_t PT_CUSTOM = 11;

uint8_t param_type_to_mav(ParamType t)
{
	switch (t) {
	case ParamType::UINT8:  return PT_UINT8;

	case ParamType::UINT32: return PT_UINT32;

	case ParamType::INT32:  return PT_INT32;

	case ParamType::REAL32: return PT_REAL32;

	case ParamType::STRING: return PT_CUSTOM;
	}

	return PT_REAL32;
}

void encode_param_value(const ParamValue &v, char out[128], uint8_t &mav_type)
{
	std::memset(out, 0, 128);
	mav_type = param_type_to_mav(v.type);

	switch (v.type) {
	case ParamType::UINT8:
		std::memcpy(out, &v.u8, 1);
		break;

	case ParamType::UINT32:
		std::memcpy(out, &v.u32, sizeof(v.u32));
		break;

	case ParamType::INT32:
		std::memcpy(out, &v.i32, sizeof(v.i32));
		break;

	case ParamType::REAL32:
		std::memcpy(out, &v.r32, sizeof(v.r32));
		break;

	case ParamType::STRING:
		std::strncpy(out, v.s.c_str(), 127);
		break;
	}
}

ParamValue decode_param_value(const char in[128], ParamType expected)
{
	ParamValue v;
	v.type = expected;

	switch (expected) {
	case ParamType::UINT8:
		std::memcpy(&v.u8, in, 1);
		break;

	case ParamType::UINT32:
		std::memcpy(&v.u32, in, sizeof(v.u32));
		break;

	case ParamType::INT32:
		std::memcpy(&v.i32, in, sizeof(v.i32));
		break;

	case ParamType::REAL32:
		std::memcpy(&v.r32, in, sizeof(v.r32));
		break;

	case ParamType::STRING: {
			char buf[129] = {};
			std::memcpy(buf, in, 128);
			v.s = buf;
			break;
		}
	}

	return v;
}

std::string param_id_to_string(const char id[16])
{
	char tmp[17] = {};
	std::memcpy(tmp, id, 16);
	return std::string(tmp);
}
} // namespace

MavlinkParametersManager::MavlinkParametersManager(Mavlink *mavlink)
	: _mavlink(mavlink)
{
	define_all_parameters();
}

void MavlinkParametersManager::init()
{
	if (!_mavlink) {
		return;
	}

	// Setup async parameters clients for target nodes
	std::vector<std::string> nodes = {"config_manager", "camera_streamer", "vision"};

	for (const auto &n : nodes) {
		_clients[n] = std::make_shared<rclcpp::AsyncParametersClient>(_mavlink, n);
	}
}

bool MavlinkParametersManager::load_schema(const std::string &schema_file)
{
	try {
		if (!std::filesystem::exists(schema_file)) {
			return false;
		}

		YAML::Node root = YAML::LoadFile(schema_file);
		if (!root["parameters"] || !root["parameters"].IsMap()) {
			return false;
		}

		_params.clear();
		_id_to_index.clear();

		for (const auto &it : root["parameters"]) {
			YAML::Node pnode = it.second;
			if (!pnode["mav_id"]) {
				continue;
			}

			std::string id = pnode["mav_id"].as<std::string>();
			std::string node = pnode["node"] ? pnode["node"].as<std::string>() : "config_manager";
			std::string ros_name = pnode["ros_name"] ? pnode["ros_name"].as<std::string>() : it.first.as<std::string>();
			std::string type_str = pnode["type"] ? pnode["type"].as<std::string>() : "string";

			ParamType ptype = ParamType::STRING;
			ParamValue val = ParamValue::Str("");

			if (type_str == "int") {
				ptype = ParamType::INT32;
				int def = pnode["default"] ? pnode["default"].as<int>() : 0;
				val = ParamValue::I32(def);
			} else if (type_str == "float") {
				ptype = ParamType::REAL32;
				float def = pnode["default"] ? pnode["default"].as<float>() : 0.0f;
				val = ParamValue::R32(def);
			} else if (type_str == "bool") {
				ptype = ParamType::UINT8;
				bool def = pnode["default"] ? pnode["default"].as<bool>() : false;
				val = ParamValue::U8(def ? 1 : 0);
			} else {
				ptype = ParamType::STRING;
				std::string def = pnode["default"] ? pnode["default"].as<std::string>() : "";
				val = ParamValue::Str(def);
			}

			_params.push_back({id, node, ros_name, ptype, val});
		}

		// Sort alphabetically by ID for stable, deterministic ordering
		std::sort(_params.begin(), _params.end(), [](const ParamEntry &a, const ParamEntry &b) {
			return a.id < b.id;
		});

		for (size_t i = 0; i < _params.size(); ++i) {
			_id_to_index[_params[i].id] = i;
		}

		return !_params.empty();
	} catch (const std::exception &e) {
		std::cerr << "[MavlinkParametersManager] Error loading schema: " << e.what() << "\n";
		return false;
	}
}

void MavlinkParametersManager::define_all_parameters()
{
	// Candidate search paths for param_schema.yaml
	const std::vector<std::string> candidates = {
		"/app/config/param_schema.yaml",
		"/etc/cc/param_schema.yaml",
		"config/param_schema.yaml",
		"../config_manager/config/param_schema.yaml",
		"src/modules/config_manager/config/param_schema.yaml",
		"Companion_Computer/src/modules/config_manager/config/param_schema.yaml"
	};

	for (const auto &p : candidates) {
		if (load_schema(p)) {
			return;
		}
	}

	_params.clear();
	_id_to_index.clear();

	// AP configuration staged on config_manager
	_params.push_back({"CC_AP_PASS", "config_manager", "ap.pass", ParamType::STRING, ParamValue::Str("")});
	_params.push_back({"CC_AP_SSID", "config_manager", "ap.ssid", ParamType::STRING, ParamValue::Str("THACO_DRONE")});

	// Camera streamer parameters
	_params.push_back({"CC_CAM_AI_EN", "camera_streamer", "ai_enabled", ParamType::UINT8, ParamValue::U8(1)});
	_params.push_back({"CC_CAM_BR", "camera_streamer", "bitrate_kbps", ParamType::UINT32, ParamValue::U32(2500)});
	_params.push_back({"CC_CAM_CODEC", "camera_streamer", "codec", ParamType::STRING, ParamValue::Str("h264")});
	_params.push_back({"CC_CAM_FPS", "camera_streamer", "fps", ParamType::UINT32, ParamValue::U32(30)});
	_params.push_back({"CC_CAM_FPV_EN", "camera_streamer", "fpv_enabled", ParamType::UINT8, ParamValue::U8(1)});
	_params.push_back({"CC_CAM_H", "camera_streamer", "height", ParamType::UINT32, ParamValue::U32(720)});
	_params.push_back({"CC_CAM_W", "camera_streamer", "width", ParamType::UINT32, ParamValue::U32(1280)});

	// 8 serial links on config_manager
	for (int i = 0; i < 8; ++i) {
		std::string s = std::to_string(i);
		_params.push_back({"CC_L" + s + "_BAUD", "config_manager", "link" + s + ".baud", ParamType::UINT32, ParamValue::U32(i == 0 ? 921600 : 115200)});
		_params.push_back({"CC_L" + s + "_NAME", "config_manager", "link" + s + ".name", ParamType::STRING, ParamValue::Str(i == 0 ? "FC" : (i == 1 ? "SIYI" : ""))});
		_params.push_back({"CC_L" + s + "_PORT", "config_manager", "link" + s + ".port", ParamType::STRING, ParamValue::Str(i == 0 ? "/dev/ttyAMA4" : (i == 1 ? "/dev/ttyAMA0" : ""))});
	}

	// Vision node parameters
	_params.push_back({"CC_VIS_CONF", "vision", "confidence", ParamType::REAL32, ParamValue::R32(0.35f)});
	_params.push_back({"CC_VIS_MODEL", "vision", "model", ParamType::STRING, ParamValue::Str("yolo26n.pt")});
	_params.push_back({"CC_VIS_SRC", "vision", "source", ParamType::STRING, ParamValue::Str("/camera/color/image_raw")});

	// Sort alphabetically by ID for stable, deterministic ordering
	std::sort(_params.begin(), _params.end(), [](const ParamEntry & a, const ParamEntry & b) {
		return a.id < b.id;
	});

	for (size_t i = 0; i < _params.size(); ++i) {
		_id_to_index[_params[i].id] = i;
	}
}

bool MavlinkParametersManager::is_node_ready(const std::string &node_name) const
{
	if (_node_ready_fn) {
		return _node_ready_fn(node_name);
	}

	auto it = _clients.find(node_name);

	if (it != _clients.end() && it->second) {
		return it->second->service_is_ready();
	}

	return false;
}

std::vector<size_t> MavlinkParametersManager::get_active_indices() const
{
	std::vector<size_t> active;
	active.reserve(_params.size());

	for (size_t i = 0; i < _params.size(); ++i) {
		if (is_node_ready(_params[i].node_name)) {
			active.push_back(i);
		}
	}

	return active;
}

size_t MavlinkParametersManager::count() const
{
	return get_active_indices().size();
}

bool MavlinkParametersManager::at(size_t active_index, ParamEntry &out) const
{
	auto active = get_active_indices();

	if (active_index < active.size()) {
		out = _params[active[active_index]];
		return true;
	}

	return false;
}

bool MavlinkParametersManager::get(const std::string &id, ParamValue &out) const
{
	auto it = _id_to_index.find(id);

	if (it != _id_to_index.end()) {
		out = _params[it->second].value;
		return true;
	}

	return false;
}

bool MavlinkParametersManager::set(const std::string &id, const ParamValue &val)
{
	auto it = _id_to_index.find(id);

	if (it == _id_to_index.end()) {
		return false;
	}

	ParamEntry &entry = _params[it->second];

	if (!is_node_ready(entry.node_name)) {
		return false;
	}

	if (_set_param_fn) {
		if (!_set_param_fn(entry.node_name, entry.ros_name, val)) {
			return false;
		}
	}

	entry.value = val;
	return true;
}

void MavlinkParametersManager::handle_request_list(const mavlink_message_t *msg)
{
	mavlink_param_ext_request_list_t m;
	mavlink_msg_param_ext_request_list_decode(msg, &m);

	if (m.target_component != _mavlink->get_comp_id() && m.target_component != 0) {
		return;
	}

	auto active = get_active_indices();

	for (size_t i = 0; i < active.size(); ++i) {
		send_param_ext_value(static_cast<uint16_t>(i), msg->sysid, msg->compid);
	}
}

void MavlinkParametersManager::handle_request_read(const mavlink_message_t *msg)
{
	mavlink_param_ext_request_read_t m;
	mavlink_msg_param_ext_request_read_decode(msg, &m);

	if (m.target_component != _mavlink->get_comp_id() && m.target_component != 0) {
		return;
	}

	auto active = get_active_indices();

	if (m.param_index >= 0) {
		if (static_cast<size_t>(m.param_index) < active.size()) {
			send_param_ext_value(static_cast<uint16_t>(m.param_index), msg->sysid, msg->compid);
		}

	} else {
		std::string id = param_id_to_string(m.param_id);

		for (size_t i = 0; i < active.size(); ++i) {
			if (_params[active[i]].id == id) {
				send_param_ext_value(static_cast<uint16_t>(i), msg->sysid, msg->compid);
				break;
			}
		}
	}
}

void MavlinkParametersManager::handle_set(const mavlink_message_t *msg)
{
	mavlink_param_ext_set_t m;
	mavlink_msg_param_ext_set_decode(msg, &m);

	if (m.target_component != _mavlink->get_comp_id() && m.target_component != 0) {
		return;
	}

	std::string id = param_id_to_string(m.param_id);
	auto it = _id_to_index.find(id);
	uint8_t result = 2; // PARAM_ACK_FAILED
	ParamValue real_val;

	if (it != _id_to_index.end()) {
		ParamEntry &entry = _params[it->second];

		if (is_node_ready(entry.node_name)) {
			ParamValue new_val = decode_param_value(m.param_value, entry.type);
			bool success = false;

			if (_set_param_fn) {
				success = _set_param_fn(entry.node_name, entry.ros_name, new_val);

			} else {
				success = true; // default accepted if no override
			}

			if (success) {
				entry.value = new_val;
				result = 0; // PARAM_ACK_ACCEPTED

			} else {
				result = 2; // PARAM_ACK_FAILED
			}

			real_val = entry.value;

		} else {
			result = 2; // absent node -> PARAM_ACK_FAILED
			real_val = entry.value;
		}

	} else {
		result = 1; // PARAM_ACK_VALUE_UNSUPPORTED
	}

	char value_buf[128];
	uint8_t mav_type = PT_REAL32;
	encode_param_value(real_val, value_buf, mav_type);

	char id_buf[16] = {};
	std::strncpy(id_buf, id.c_str(), sizeof(id_buf) - 1);

	mavlink_message_t ack;
	mavlink_msg_param_ext_ack_pack(
		_mavlink->get_sys_id(), _mavlink->get_comp_id(), &ack,
		id_buf, value_buf, mav_type, result);
	_mavlink->send_message(&ack);

	// Also push current value
	auto active = get_active_indices();

	for (size_t i = 0; i < active.size(); ++i) {
		if (_params[active[i]].id == id) {
			send_param_ext_value(static_cast<uint16_t>(i), msg->sysid, msg->compid);
			break;
		}
	}
}

void MavlinkParametersManager::send_param_ext_value(uint16_t index, uint8_t target_sys, uint8_t target_comp)
{
	(void)target_sys;
	(void)target_comp;
	auto active = get_active_indices();

	if (index >= active.size()) {
		return;
	}

	const ParamEntry &entry = _params[active[index]];
	char value_buf[128];
	uint8_t mav_type = PT_REAL32;
	encode_param_value(entry.value, value_buf, mav_type);

	char id_buf[16] = {};
	std::strncpy(id_buf, entry.id.c_str(), sizeof(id_buf) - 1);

	mavlink_message_t msg;
	mavlink_msg_param_ext_value_pack(
		_mavlink->get_sys_id(), _mavlink->get_comp_id(), &msg,
		id_buf, value_buf, mav_type,
		static_cast<uint16_t>(active.size()), index);
	_mavlink->send_message(&msg);
}
