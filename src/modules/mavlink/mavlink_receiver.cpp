#include "mavlink_receiver.h"
#include "mavlink_main.h"
#include "mavlink_parameters.h"
#include "mavlink_bridge_header.h"
#include <hrt/hrt.hpp>

MavlinkReceiver::MavlinkReceiver(Mavlink *mavlink, MavlinkParametersManager *parameters_manager)
	: _mavlink(mavlink), _parameters_manager(parameters_manager)
{
	if (_mavlink) {
		// Use transient_local QoS for vehicle_status so late joiners get latest state
		rclcpp::QoS qos_status(1);
		qos_status.transient_local();
		_vehicle_status_pub = _mavlink->create_publisher<cc_msgs::msg::VehicleStatus>("/cc/vehicle_status", qos_status);
		_vehicle_command_pub = _mavlink->create_publisher<cc_msgs::msg::VehicleCommand>("/cc/vehicle_command", 10);
	}
}

void MavlinkReceiver::handle_message(const mavlink_message_t *msg)
{
	switch (msg->msgid) {
	case MAVLINK_MSG_ID_HEARTBEAT:
		handle_message_heartbeat(msg);
		break;

	case MAVLINK_MSG_ID_COMMAND_LONG:
		handle_message_command_long(msg);
		break;

	case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST:
		if (_parameters_manager) {
			_parameters_manager->handle_request_list(msg);
		}

		break;

	case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ:
		if (_parameters_manager) {
			_parameters_manager->handle_request_read(msg);
		}

		break;

	case MAVLINK_MSG_ID_PARAM_EXT_SET:
		if (_parameters_manager) {
			_parameters_manager->handle_set(msg);
		}

		break;

	case MAVLINK_MSG_ID_CC_TELEMETRY_LINKS:
		handle_message_cc_telemetry_links(msg);
		break;

	case MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA:
		handle_message_cc_telemetry_camera(msg);
		break;

	case MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK:
		handle_message_cc_telemetry_network(msg);
		break;

	case MAVLINK_MSG_ID_CC_TELEMETRY_VISION:
		handle_message_cc_telemetry_vision(msg);
		break;

	default:
		break;
	}
}

void MavlinkReceiver::handle_message_heartbeat(const mavlink_message_t *msg)
{
	mavlink_heartbeat_t hb;
	mavlink_msg_heartbeat_decode(msg, &hb);

	// Autopilot only: same sysid, compid MAV_COMP_ID_AUTOPILOT1 (1), autopilot != INVALID
	if (msg->sysid == _mavlink->get_sys_id() &&
	    msg->compid == MAV_COMP_ID_AUTOPILOT1 &&
	    hb.autopilot != MAV_AUTOPILOT_INVALID) {
		cc_msgs::msg::VehicleStatus status;
		status.timestamp = hrt_absolute_time();
		status.arming_state = (hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) ?
				      cc_msgs::msg::VehicleStatus::ARMING_STATE_ARMED :
				      cc_msgs::msg::VehicleStatus::ARMING_STATE_DISARMED;

		if (_vehicle_status_pub) {
			_vehicle_status_pub->publish(status);
		}
	}
}

void MavlinkReceiver::handle_message_command_long(const mavlink_message_t *msg)
{
	mavlink_command_long_t cmd;
	mavlink_msg_command_long_decode(msg, &cmd);

	// Only publish commands targeted to 191 (onboard computer) or 0 (broadcast)
	if (cmd.target_component == _mavlink->get_comp_id() || cmd.target_component == 0) {
		cc_msgs::msg::VehicleCommand vcmd;
		vcmd.timestamp = hrt_absolute_time();
		vcmd.command = cmd.command;
		vcmd.param1 = cmd.param1;
		vcmd.param2 = cmd.param2;
		vcmd.param3 = cmd.param3;
		vcmd.param4 = cmd.param4;
		vcmd.param5 = cmd.param5;
		vcmd.param6 = cmd.param6;
		vcmd.param7 = cmd.param7;
		vcmd.target_system = cmd.target_system;
		vcmd.target_component = cmd.target_component;
		vcmd.source_system = msg->sysid;
		vcmd.source_component = msg->compid;
		vcmd.confirmation = cmd.confirmation;
		vcmd.from_external = true;

		if (_vehicle_command_pub) {
			_vehicle_command_pub->publish(vcmd);
		}
	}
}

void MavlinkReceiver::handle_message_cc_telemetry_links(const mavlink_message_t *msg)
{
	if (!_parameters_manager) {
		return;
	}

	mavlink_cc_telemetry_links_t m;
	mavlink_msg_cc_telemetry_links_decode(msg, &m);

	_parameters_manager->set("CC_L0_PORT", ParamValue::Str(m.fc_port));
	_parameters_manager->set("CC_L0_BAUD", ParamValue::U32(m.fc_baudrate));
	_parameters_manager->set("CC_L1_PORT", ParamValue::Str(m.siyi_port));
	_parameters_manager->set("CC_L1_BAUD", ParamValue::U32(m.siyi_baudrate));
}

void MavlinkReceiver::handle_message_cc_telemetry_camera(const mavlink_message_t *msg)
{
	if (!_parameters_manager) {
		return;
	}

	mavlink_cc_telemetry_camera_t m;
	mavlink_msg_cc_telemetry_camera_decode(msg, &m);

	_parameters_manager->set("CC_CAM_W", ParamValue::U32(m.video_width));
	_parameters_manager->set("CC_CAM_H", ParamValue::U32(m.video_height));
	_parameters_manager->set("CC_CAM_FPS", ParamValue::U32(m.video_fps));
	_parameters_manager->set("CC_CAM_BR", ParamValue::U32(m.bitrate_kbps));
	_parameters_manager->set("CC_CAM_CODEC", ParamValue::Str(m.codec));
}

void MavlinkReceiver::handle_message_cc_telemetry_network(const mavlink_message_t *msg)
{
	if (!_parameters_manager) {
		return;
	}

	mavlink_cc_telemetry_network_t m;
	mavlink_msg_cc_telemetry_network_decode(msg, &m);

	_parameters_manager->set("CC_AP_SSID", ParamValue::Str(m.ap_ssid));
	_parameters_manager->set("CC_AP_PASS", ParamValue::Str(m.ap_wpa_passphrase));
}

void MavlinkReceiver::handle_message_cc_telemetry_vision(const mavlink_message_t *msg)
{
	if (!_parameters_manager) {
		return;
	}

	mavlink_cc_telemetry_vision_t m;
	mavlink_msg_cc_telemetry_vision_decode(msg, &m);

	_parameters_manager->set("CC_VIS_MODEL", ParamValue::Str(m.model_name));
	_parameters_manager->set("CC_VIS_CONF", ParamValue::R32(m.confidence_thresh));
	_parameters_manager->set("CC_VIS_SRC", ParamValue::Str(m.input_source));
}
