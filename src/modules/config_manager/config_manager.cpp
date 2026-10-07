#include "config_manager.h"

#include <chrono>
#include <systemlib/mavlink_log.h>
#include <hrt/hrt.hpp>

using namespace std::chrono_literals;

namespace cc
{

ConfigManager::ConfigManager(const rclcpp::NodeOptions &node_options, const ConfigManagerOptions &options)
	: Node("config_manager", node_options),
	  _options(options),
	  _link_store(options.link_paths),
	  _hotspot_store(options.hotspot_paths)
{
	if (_options.port_check_fn) {
		_link_store.set_port_check_fn(_options.port_check_fn);
	}

	if (_options.router_apply_fn) {
		_link_store.set_router_apply_fn(_options.router_apply_fn);

	} else {
		_link_store.set_router_apply_fn([this](const Links & links, std::string & err) {
			return apply_via_router_service(links, err);
		});
	}

	_cmd_callback_group = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
	_client_callback_group = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

	// Setup publishers
	_pub_vehicle_command_ack = create_publisher<cc_msgs::msg::VehicleCommandAck>("/cc/vehicle_command_ack", 10);
	_pub_link_config = create_publisher<cc_msgs::msg::LinkConfig>("/cc/link_config", rclcpp::QoS(1).transient_local().reliable());
	_pub_mavlink_log = create_publisher<cc_msgs::msg::MavlinkLog>("/cc/mavlink_log", 10);
	_pub_system_status = create_publisher<cc_msgs::msg::SystemStatus>("/cc/system_status", 10);

	// Setup subscriptions
	rclcpp::SubscriptionOptions sub_opts;
	sub_opts.callback_group = _cmd_callback_group;

	_sub_vehicle_command = create_subscription<cc_msgs::msg::VehicleCommand>(
				       "/cc/vehicle_command", 10,
	[this](const cc_msgs::msg::VehicleCommand::SharedPtr msg) {
		handle_vehicle_command(msg);
	}, sub_opts);

	_sub_vehicle_status = create_subscription<cc_msgs::msg::VehicleStatus>(
				      "/cc/vehicle_status", rclcpp::QoS(1).transient_local(),
	[this](const cc_msgs::msg::VehicleStatus::SharedPtr msg) {
		handle_vehicle_status(msg);
	});

	_sub_serial_link_status = create_subscription<cc_msgs::msg::SerialLinkStatus>(
					  "/cc/serial_link_status", 10,
	[this](const cc_msgs::msg::SerialLinkStatus::SharedPtr msg) {
		handle_serial_link_status(msg);
	});

	// Setup client for router apply_links
	_router_client = create_client<cc_msgs::srv::ApplyLinks>(
				 "/cc/router/apply_links", rclcpp::ServicesQoS(), _client_callback_group);

	// Initialize parameters
	init_parameters();

	// LoadMon timer (1 Hz)
	if (_options.enable_load_mon) {
		_load_mon_timer = create_wall_timer(1s, [this]() {
			publish_system_status();
		});
	}
}

void ConfigManager::init_parameters()
{
	auto active_links = _link_store.active();

	for (int i = 0; i < 8; ++i) {
		std::string prefix = "link" + std::to_string(i) + ".";

		rcl_interfaces::msg::ParameterDescriptor name_desc;
		name_desc.description = "Serial link name";
		std::string def_name = (i < static_cast<int>(active_links.size())) ? active_links[i].name : "";
		declare_parameter(prefix + "name", def_name, name_desc);

		rcl_interfaces::msg::ParameterDescriptor port_desc;
		port_desc.description = "Serial link device port path";
		std::string def_port = (i < static_cast<int>(active_links.size())) ? active_links[i].port : "";
		declare_parameter(prefix + "port", def_port, port_desc);

		rcl_interfaces::msg::ParameterDescriptor baud_desc;
		baud_desc.description = "Serial link baud rate";
		int64_t def_baud = (i < static_cast<int>(active_links.size())) ? static_cast<int64_t>(active_links[i].baud) : 0;
		declare_parameter(prefix + "baud", def_baud, baud_desc);
	}

	rcl_interfaces::msg::ParameterDescriptor ssid_desc;
	ssid_desc.description = "Access point SSID";
	declare_parameter("ap.ssid", _hotspot_store.effective_ssid(), ssid_desc);

	rcl_interfaces::msg::ParameterDescriptor pass_desc;
	pass_desc.description = "Access point WPA2 passphrase";
	declare_parameter("ap.pass", _hotspot_store.effective_password(), pass_desc);

	_params_callback_handle = add_on_set_parameters_callback(
	[this](const std::vector<rclcpp::Parameter> &params) {
		return on_set_parameters(params);
	});
}

rcl_interfaces::msg::SetParametersResult ConfigManager::on_set_parameters(const std::vector<rclcpp::Parameter> &params)
{
	rcl_interfaces::msg::SetParametersResult result;
	result.successful = true;

	std::vector<std::string> avail_ports;
	{
		std::lock_guard<std::mutex> l(_ports_mu);
		avail_ports = _available_ports;
	}

	for (const auto &p : params) {
		const auto &name = p.get_name();

		if (name.rfind("link", 0) == 0) {
			if (name.ends_with(".name")) {
				if (p.get_type() != rclcpp::ParameterType::PARAMETER_STRING) {
					result.successful = false;
					result.reason = "name must be string";
					return result;
				}

				const auto val = p.as_string();

				if (!val.empty() && !is_valid_link_name(val)) {
					result.successful = false;
					result.reason = "invalid link name: " + val;
					return result;
				}

			} else if (name.ends_with(".port")) {
				if (p.get_type() != rclcpp::ParameterType::PARAMETER_STRING) {
					result.successful = false;
					result.reason = "port must be string";
					return result;
				}

				const auto val = p.as_string();

				if (!val.empty()) {
					if (!is_valid_port_path(val)) {
						result.successful = false;
						result.reason = "invalid port path: " + val;
						return result;
					}

					if (!avail_ports.empty()) {
						bool found = false;

						for (const auto &ap : avail_ports) {
							if (ap == val) {
								found = true;
								break;
							}
						}

						if (!found) {
							result.successful = false;
							result.reason = "port not in available_ports: " + val;
							return result;
						}
					}
				}

			} else if (name.ends_with(".baud")) {
				if (p.get_type() != rclcpp::ParameterType::PARAMETER_INTEGER) {
					result.successful = false;
					result.reason = "baud must be integer";
					return result;
				}

				int64_t val = p.as_int();

				if (val > 0 && !is_standard_baud(static_cast<uint32_t>(val))) {
					result.successful = false;
					result.reason = "invalid baud rate: " + std::to_string(val);
					return result;
				}
			}

		} else if (name == "ap.ssid") {
			if (p.get_type() != rclcpp::ParameterType::PARAMETER_STRING) {
				result.successful = false;
				result.reason = "ssid must be string";
				return result;
			}

			const auto val = p.as_string();

			if (!val.empty() && !is_valid_ssid(val)) {
				result.successful = false;
				result.reason = "invalid SSID";
				return result;
			}

		} else if (name == "ap.pass") {
			if (p.get_type() != rclcpp::ParameterType::PARAMETER_STRING) {
				result.successful = false;
				result.reason = "passphrase must be string";
				return result;
			}

			const auto val = p.as_string();

			if (!val.empty() && !is_valid_wpa_passphrase(val)) {
				result.successful = false;
				result.reason = "WPA2 passphrase must be 8..63 printable ASCII";
				return result;
			}
		}
	}

	return result;
}

void ConfigManager::handle_vehicle_status(const cc_msgs::msg::VehicleStatus::SharedPtr msg)
{
	_arming_latch.update_vehicle_status(msg->arming_state);
}

void ConfigManager::handle_serial_link_status(const cc_msgs::msg::SerialLinkStatus::SharedPtr msg)
{
	std::lock_guard<std::mutex> l(_ports_mu);
	_available_ports.assign(msg->available_ports.begin(), msg->available_ports.end());
}

void ConfigManager::publish_ack(const cc_msgs::msg::VehicleCommand &cmd, uint8_t result)
{
	cc_msgs::msg::VehicleCommandAck ack;
	ack.timestamp = hrt_absolute_time();
	ack.command = cmd.command;
	ack.result = result;
	ack.result_param1 = 0;
	ack.result_param2 = 0;
	ack.target_system = cmd.source_system;
	ack.target_component = cmd.source_component;
	ack.from_external = cmd.from_external;
	_pub_vehicle_command_ack->publish(ack);
}

void ConfigManager::publish_link_config(const Links &links)
{
	cc_msgs::msg::LinkConfig msg;
	msg.timestamp = hrt_absolute_time();

	for (const auto &l : links) {
		cc_msgs::msg::Link link_msg;
		link_msg.name = l.name;
		link_msg.port = l.port;
		link_msg.baud = l.baud;
		msg.links.push_back(link_msg);
	}

	_pub_link_config->publish(msg);
}

void ConfigManager::publish_system_status()
{
	auto m = _load_mon.read_metrics();
	cc_msgs::msg::SystemStatus msg;
	msg.timestamp = hrt_absolute_time();
	msg.system_uptime_s = m.uptime_s;
	msg.cpu_usage = m.cpu_usage;
	msg.ram_usage = m.ram_usage;
	msg.disk_usage = m.disk_usage;
	msg.cpu_temp = m.cpu_temp;
	msg.system_status = 4; // MAV_STATE_ACTIVE
	_pub_system_status->publish(msg);
}

bool ConfigManager::router_service_is_ready()
{
	if (_options.router_ready_fn) {
		return _options.router_ready_fn();
	}

	return _router_client && _router_client->service_is_ready();
}

bool ConfigManager::apply_via_router_service(const Links &links, std::string &err)
{
	if (!_router_client) {
		err = "router client not initialized";
		return false;
	}

	auto req = std::make_shared<cc_msgs::srv::ApplyLinks::Request>();
	req->config.timestamp = hrt_absolute_time();

	for (const auto &l : links) {
		cc_msgs::msg::Link link_msg;
		link_msg.name = l.name;
		link_msg.port = l.port;
		link_msg.baud = l.baud;
		req->config.links.push_back(link_msg);
	}

	auto future = _router_client->async_send_request(req);
	auto status = future.wait_for(5s);

	if (status != std::future_status::ready) {
		err = "router service timeout (5s)";
		return false;
	}

	auto resp = future.get();

	if (!resp->success) {
		err = resp->message;
		return false;
	}

	return true;
}

Links ConfigManager::get_links_from_parameters()
{
	Links links;

	for (int i = 0; i < 8; ++i) {
		std::string prefix = "link" + std::to_string(i) + ".";
		std::string name = get_parameter(prefix + "name").as_string();
		std::string port = get_parameter(prefix + "port").as_string();
		int64_t baud = get_parameter(prefix + "baud").as_int();

		if (!port.empty()) {
			links.push_back({name, port, static_cast<uint32_t>(baud)});
		}
	}

	return links;
}

void ConfigManager::sync_parameters_from_links(const Links &links)
{
	for (int i = 0; i < 8; ++i) {
		std::string prefix = "link" + std::to_string(i) + ".";

		if (i < static_cast<int>(links.size())) {
			set_parameter(rclcpp::Parameter(prefix + "name", links[i].name));
			set_parameter(rclcpp::Parameter(prefix + "port", links[i].port));
			set_parameter(rclcpp::Parameter(prefix + "baud", static_cast<int64_t>(links[i].baud)));

		} else {
			set_parameter(rclcpp::Parameter(prefix + "name", ""));
			set_parameter(rclcpp::Parameter(prefix + "port", ""));
			set_parameter(rclcpp::Parameter(prefix + "baud", static_cast<int64_t>(0)));
		}
	}
}

void ConfigManager::handle_vehicle_command(const cc_msgs::msg::VehicleCommand::SharedPtr msg)
{
	if (msg->target_component != 191 && msg->target_component != 0) {
		return;
	}

	const uint32_t cmd = msg->command;
	constexpr uint32_t CMD_SAVE = cc_msgs::msg::VehicleCommand::VEHICLE_CMD_THACO_SAVE_DEFAULT_CONFIG;
	constexpr uint32_t CMD_APPLY = cc_msgs::msg::VehicleCommand::VEHICLE_CMD_THACO_APPLY_CONFIG;
	constexpr uint32_t CMD_RESTORE = cc_msgs::msg::VehicleCommand::VEHICLE_CMD_THACO_RESTORE_DEFAULT_CONFIG;

	if (cmd != CMD_SAVE && cmd != CMD_APPLY && cmd != CMD_RESTORE) {
		if (msg->target_component == 191) {
			publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);
		}

		// Broadcast with unknown command -> ignored without ACK
		return;
	}

	// Always publish IN_PROGRESS first
	publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);

	const uint32_t sub = static_cast<uint32_t>(msg->param1);

	// Check ArmingLatch: APPLY (44011) or RESTORE (44012) on sub 0 or 1
	const bool touches_links = (sub == 0 || sub == 1);
	const bool modifies_active_links = (cmd == CMD_APPLY || cmd == CMD_RESTORE);

	if (touches_links && modifies_active_links && !_arming_latch.can_apply_links()) {
		mavlink_log_critical(_pub_mavlink_log, "APPLY LINKS rejected: vehicle is armed");
		publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);
		return;
	}

	std::string err;

	switch (sub) {
	case 1: { // LINKS
			if (cmd == CMD_APPLY) {
				auto links = get_links_from_parameters();

				if (!router_service_is_ready()) {
					mavlink_log_critical(_pub_mavlink_log, "router not ready");
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);
					return;
				}

				auto res = _link_store.apply(links, err);

				if (res == ApplyResult::Ok) {
					publish_link_config(_link_store.active());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "APPLY LINKS failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}

			} else if (cmd == CMD_SAVE) {
				if (_link_store.save_default(err)) {
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "SAVE LINKS failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}

			} else if (cmd == CMD_RESTORE) {
				if (!router_service_is_ready()) {
					mavlink_log_critical(_pub_mavlink_log, "router not ready");
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);
					return;
				}

				auto res = _link_store.restore_default(err);

				if (res == ApplyResult::Ok) {
					sync_parameters_from_links(_link_store.active());
					publish_link_config(_link_store.active());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "RESTORE LINKS failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}
			}

			break;
		}

	case 2: { // CAMERA
			if (cmd == CMD_APPLY) {
				// Camera parameters bitrate/fps applied live
				publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

			} else {
				// SAVE and RESTORE for camera are UNSUPPORTED per spec §5.8
				publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);
			}

			break;
		}

	case 3: { // HOTSPOT (Network) - NOT locked when armed
			if (cmd == CMD_APPLY) {
				std::string ssid = get_parameter("ap.ssid").as_string();
				std::string pass = get_parameter("ap.pass").as_string();

				if (_hotspot_store.apply(ssid, pass, err)) {
					if (_options.networking_reload_fn) {
						_options.networking_reload_fn();
					}

					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "APPLY HOTSPOT failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}

			} else if (cmd == CMD_SAVE) {
				if (_hotspot_store.save_default(err)) {
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "SAVE HOTSPOT failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}

			} else if (cmd == CMD_RESTORE) {
				if (_hotspot_store.restore_default(err)) {
					set_parameter(rclcpp::Parameter("ap.ssid", _hotspot_store.effective_ssid()));
					set_parameter(rclcpp::Parameter("ap.pass", _hotspot_store.effective_password()));

					if (_options.networking_reload_fn) {
						_options.networking_reload_fn();
					}

					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "RESTORE HOTSPOT failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}
			}

			break;
		}

	case 4: { // VISION
			// UNSUPPORTED per spec §5.8
			publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);
			break;
		}

	case 0: { // ALL
			if (cmd == CMD_APPLY) {
				auto links = get_links_from_parameters();

				if (!router_service_is_ready()) {
					mavlink_log_critical(_pub_mavlink_log, "router not ready");
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);
					return;
				}

				auto res = _link_store.apply(links, err);

				if (res != ApplyResult::Ok) {
					mavlink_log_critical(_pub_mavlink_log, "APPLY ALL (links) failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
					return;
				}

				publish_link_config(_link_store.active());

				// Hotspot
				std::string ssid = get_parameter("ap.ssid").as_string();
				std::string pass = get_parameter("ap.pass").as_string();

				if (!_hotspot_store.apply(ssid, pass, err)) {
					mavlink_log_critical(_pub_mavlink_log, "APPLY ALL (hotspot) failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
					return;
				}

				if (_options.networking_reload_fn) {
					_options.networking_reload_fn();
				}

				publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

			} else if (cmd == CMD_SAVE) {
				bool ok_links = _link_store.save_default(err);
				bool ok_hotspot = _hotspot_store.save_default(err);

				if (ok_links && ok_hotspot) {
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

				} else {
					mavlink_log_critical(_pub_mavlink_log, "SAVE ALL failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
				}

			} else if (cmd == CMD_RESTORE) {
				if (!router_service_is_ready()) {
					mavlink_log_critical(_pub_mavlink_log, "router not ready");
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);
					return;
				}

				auto res = _link_store.restore_default(err);

				if (res != ApplyResult::Ok) {
					mavlink_log_critical(_pub_mavlink_log, "RESTORE ALL (links) failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
					return;
				}

				sync_parameters_from_links(_link_store.active());
				publish_link_config(_link_store.active());

				if (!_hotspot_store.restore_default(err)) {
					mavlink_log_critical(_pub_mavlink_log, "RESTORE ALL (hotspot) failed: %s", err.c_str());
					publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_FAILED);
					return;
				}

				set_parameter(rclcpp::Parameter("ap.ssid", _hotspot_store.effective_ssid()));
				set_parameter(rclcpp::Parameter("ap.pass", _hotspot_store.effective_password()));

				if (_options.networking_reload_fn) {
					_options.networking_reload_fn();
				}

				publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);
			}

			break;
		}

	default:
		publish_ack(*msg, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);
		break;
	}
}

} // namespace cc
