#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/vision_status.hpp>
#include <hrt/hrt.hpp>
#include <cstring>

class MavlinkStreamCcTelemetryVision : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamCcTelemetryVision(mavlink); }
	static constexpr const char *get_name_static() { return "CC_TELEMETRY_VISION"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_CC_TELEMETRY_VISION; }

	explicit MavlinkStreamCcTelemetryVision(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::VisionStatus>("/cc/vision_status"))
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		rclcpp::MessageInfo info;

		if (_sub) {
			_sub->take(_status, info);
		}

		if (_status.timestamp == 0 || (hrt_absolute_time() - _status.timestamp > Mavlink::STALE_US)) {
			return false;
		}

		mavlink_cc_telemetry_vision_t msg{};
		msg.confidence_thresh = _status.confidence_thresh;
		msg.inference_fps = _status.inference_fps;
		msg.input_width = _status.input_width;
		msg.input_height = _status.input_height;
		msg.video_fps = _status.video_fps;
		msg.detections_count = _status.detections_count;
		msg.status_flags = _status.status_flags;

		std::strncpy(msg.model_name, _status.model_name.c_str(), sizeof(msg.model_name) - 1);
		std::strncpy(msg.input_source, _status.input_source.c_str(), sizeof(msg.input_source) - 1);

		mavlink_msg_cc_telemetry_vision_send_struct(_mavlink->get_channel(), &msg);
		return true;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::VisionStatus>::SharedPtr _sub;
	cc_msgs::msg::VisionStatus _status{};
};
