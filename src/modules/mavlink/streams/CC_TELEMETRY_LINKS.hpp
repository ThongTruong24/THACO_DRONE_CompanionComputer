#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/serial_link_status.hpp>
#include <hrt/hrt.hpp>
#include <cstring>

class MavlinkStreamCcTelemetryLinks : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamCcTelemetryLinks(mavlink); }
	static constexpr const char *get_name_static() { return "CC_TELEMETRY_LINKS"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_CC_TELEMETRY_LINKS; }

	explicit MavlinkStreamCcTelemetryLinks(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::SerialLinkStatus>("/cc/serial_link_status"))
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		rclcpp::MessageInfo info;

		if (_sub) {
			_sub->take(_status, info);
		}

		if (_status.timestamp == 0 || (hrt_absolute_time() - _status.timestamp > Mavlink::STALE_US)) {
			return false;
		}

		mavlink_cc_telemetry_links_t msg{};
		std::string ports_joined;

		for (size_t i = 0; i < _status.available_ports.size(); ++i) {
			if (i > 0) {
				ports_joined += ",";
			}

			ports_joined += _status.available_ports[i];
		}

		std::strncpy(msg.available_ports, ports_joined.c_str(), sizeof(msg.available_ports) - 1);

		if (!_status.links.empty()) {
			const auto &l0 = _status.links[0];
			msg.fc_tx_rate = l0.tx_rate;
			msg.fc_rx_rate = l0.rx_rate;
			msg.fc_rx_loss = l0.rx_loss;
			msg.fc_tx_err = l0.rx_errors;
			msg.fc_bytes_rx = l0.rx_bytes;
			msg.fc_bytes_tx = l0.tx_bytes;
			msg.fc_baudrate = l0.baudrate;
			msg.fc_status = l0.status;
			std::strncpy(msg.fc_port, l0.port.c_str(), sizeof(msg.fc_port) - 1);
		}

		if (_status.links.size() > 1) {
			const auto &l1 = _status.links[1];
			msg.siyi_tx_rate = l1.tx_rate;
			msg.siyi_rx_rate = l1.rx_rate;
			msg.siyi_rx_loss = l1.rx_loss;
			msg.siyi_tx_err = l1.rx_errors;
			msg.siyi_bytes_rx = l1.rx_bytes;
			msg.siyi_bytes_tx = l1.tx_bytes;
			msg.siyi_baudrate = l1.baudrate;
			msg.siyi_status = l1.status;
			std::strncpy(msg.siyi_port, l1.port.c_str(), sizeof(msg.siyi_port) - 1);
		}

		mavlink_msg_cc_telemetry_links_send_struct(_mavlink->get_channel(), &msg);
		return true;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::SerialLinkStatus>::SharedPtr _sub;
	cc_msgs::msg::SerialLinkStatus _status{};
};
