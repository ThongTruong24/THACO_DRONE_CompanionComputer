#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/camera_status.hpp>
#include <hrt/hrt.hpp>
#include <cstring>

class MavlinkStreamCcTelemetryCamera : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamCcTelemetryCamera(mavlink); }
	static constexpr const char *get_name_static() { return "CC_TELEMETRY_CAMERA"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA; }

	explicit MavlinkStreamCcTelemetryCamera(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::CameraStatus>("/cc/camera_status"))
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		rclcpp::MessageInfo info;

		if (_sub) {
			_sub->take(_status, info);
		}

		if (_status.timestamp == 0 || (hrt_absolute_time() - _status.timestamp > Mavlink::STALE_US)) {
			return false;
		}

		mavlink_cc_telemetry_camera_t msg{};
		msg.video_width = _status.video_width;
		msg.video_height = _status.video_height;
		msg.depth_width = _status.depth_width;
		msg.depth_height = _status.depth_height;
		msg.rotation = _status.rotation;
		msg.bitrate_kbps = _status.bitrate_kbps;
		msg.bitrate_max_kbps = _status.bitrate_max_kbps;
		msg.vbv_buffer_kb = _status.vbv_buffer_kb;
		msg.rtsp_port = _status.rtsp_port;
		msg.video_fps = _status.video_fps;
		msg.depth_fps = _status.depth_fps;
		msg.camera_id = _status.camera_id;
		msg.is_default = _status.is_default;
		msg.profile_mode = _status.profile_mode;
		msg.enable_emitter = _status.enable_emitter;
		msg.camera_status = _status.camera_status;
		msg.error_code = _status.error_code;
		msg.usb_speed_mode = _status.usb_speed_mode;

		std::strncpy(msg.camera_name, _status.camera_name.c_str(), sizeof(msg.camera_name) - 1);
		std::strncpy(msg.camera_type, _status.camera_type.c_str(), sizeof(msg.camera_type) - 1);
		std::strncpy(msg.connection_port, _status.connection_port.c_str(), sizeof(msg.connection_port) - 1);
		std::strncpy(msg.serial_number, _status.serial_number.c_str(), sizeof(msg.serial_number) - 1);
		std::strncpy(msg.codec, _status.codec.c_str(), sizeof(msg.codec) - 1);
		std::strncpy(msg.encoder_mode, _status.encoder_mode.c_str(), sizeof(msg.encoder_mode) - 1);
		std::strncpy(msg.rtsp_url_qgc, _status.rtsp_url_qgc.c_str(), sizeof(msg.rtsp_url_qgc) - 1);
		std::strncpy(msg.rtsp_url_controller, _status.rtsp_url_controller.c_str(), sizeof(msg.rtsp_url_controller) - 1);
		std::strncpy(msg.rtsp_url_laptop, _status.rtsp_url_laptop.c_str(), sizeof(msg.rtsp_url_laptop) - 1);

		mavlink_msg_cc_telemetry_camera_send_struct(_mavlink->get_channel(), &msg);
		return true;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::CameraStatus>::SharedPtr _sub;
	cc_msgs::msg::CameraStatus _status{};
};
