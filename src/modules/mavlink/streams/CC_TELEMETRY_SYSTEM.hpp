#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/system_status.hpp>
#include <hrt/hrt.hpp>
#include <cstring>

class MavlinkStreamCcTelemetrySystem : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamCcTelemetrySystem(mavlink); }
	static constexpr const char *get_name_static() { return "CC_TELEMETRY_SYSTEM"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM; }

	explicit MavlinkStreamCcTelemetrySystem(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::SystemStatus>("/cc/system_status"))
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		rclcpp::MessageInfo info;

		if (_sub) {
			_sub->take(_status, info);
		}

		if (_status.timestamp == 0 || (hrt_absolute_time() - _status.timestamp > Mavlink::STALE_US)) {
			return false;
		}

		mavlink_cc_telemetry_system_t msg{};
		msg.system_uptime_s = _status.system_uptime_s;
		msg.cpu_usage = _status.cpu_usage;
		msg.ram_usage = _status.ram_usage;
		msg.disk_usage = _status.disk_usage;
		msg.cpu_temp = _status.cpu_temp;
		msg.system_status = _status.system_status;

		mavlink_msg_cc_telemetry_system_send_struct(_mavlink->get_channel(), &msg);
		return true;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::SystemStatus>::SharedPtr _sub;
	cc_msgs::msg::SystemStatus _status{};
};
