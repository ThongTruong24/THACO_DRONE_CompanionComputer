#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/mavlink_log.hpp>
#include <cstring>

class MavlinkStreamStatustext : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamStatustext(mavlink); }
	static constexpr const char *get_name_static() { return "STATUSTEXT"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_STATUSTEXT; }

	explicit MavlinkStreamStatustext(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::MavlinkLog>("/cc/mavlink_log", 20))
	{
		set_interval(50000); // 20 Hz (50 ms)
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_STATUSTEXT_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		cc_msgs::msg::MavlinkLog log_msg;
		rclcpp::MessageInfo info;
		bool sent_any = false;

		while (_sub && _sub->take(log_msg, info)) {
			mavlink_statustext_t st{};
			st.severity = log_msg.severity;
			std::strncpy(st.text, log_msg.text.c_str(), sizeof(st.text) - 1);
			mavlink_msg_statustext_send_struct(_mavlink->get_channel(), &st);
			sent_any = true;
		}

		return sent_any;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::MavlinkLog>::SharedPtr _sub;
};
