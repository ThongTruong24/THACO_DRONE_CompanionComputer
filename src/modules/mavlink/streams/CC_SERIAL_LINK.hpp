#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/serial_link_status.hpp>
#include <hrt/hrt.hpp>
#include <cstring>

class MavlinkStreamCcSerialLink : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamCcSerialLink(mavlink); }
	static constexpr const char *get_name_static() { return "CC_SERIAL_LINK"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_CC_SERIAL_LINK; }

	explicit MavlinkStreamCcSerialLink(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::SerialLinkStatus>("/cc/serial_link_status"))
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		rclcpp::MessageInfo info;

		if (_sub) {
			_sub->take(_status, info);
		}

		if (_status.timestamp == 0 || (hrt_absolute_time() - _status.timestamp > Mavlink::STALE_US)) {
			return false;
		}

		for (const auto &link : _status.links) {
			mavlink_cc_serial_link_t msg{};
			msg.rx_rate = link.rx_rate;
			msg.tx_rate = link.tx_rate;
			msg.rx_loss = link.rx_loss;
			msg.rx_bytes = link.rx_bytes;
			msg.tx_bytes = link.tx_bytes;
			msg.rx_errors = link.rx_errors;
			msg.baudrate = link.baudrate;
			msg.link_index = link.link_index;
			msg.link_count = link.link_count;
			msg.status = link.status;
			std::strncpy(msg.name, link.name.c_str(), sizeof(msg.name) - 1);
			std::strncpy(msg.port, link.port.c_str(), sizeof(msg.port) - 1);
			mavlink_msg_cc_serial_link_send_struct(_mavlink->get_channel(), &msg);
		}

		return true;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::SerialLinkStatus>::SharedPtr _sub;
	cc_msgs::msg::SerialLinkStatus _status{};
};
