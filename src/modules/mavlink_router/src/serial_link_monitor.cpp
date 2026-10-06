#include "mavlink_router/serial_link_monitor.hpp"
#include <hrt/hrt.hpp>

namespace cc
{

void SerialLinkMonitor::reset()
{
	hold_ticks_.clear();
	last_status_.clear();
}

cc_msgs::msg::SerialLinkStatus SerialLinkMonitor::tick(
	const Links &active_links,
	const std::map<std::string, RouterEndpointStat> &router_stats,
	ISerialStats &serial_stats,
	const std::vector<std::string> &available_ports,
	std::vector<std::string> &out_transition_logs)
{
	cc_msgs::msg::SerialLinkStatus status_msg;
	status_msg.timestamp = hrt_absolute_time();
	const uint8_t count = static_cast<uint8_t>(active_links.size());

	for (const auto &p : available_ports) {
		if (status_msg.available_ports.size() < 32) {
			status_msg.available_ports.push_back(p);
		}
	}

	for (size_t i = 0; i < active_links.size(); ++i) {
		const auto &l = active_links[i];
		cc_msgs::msg::SerialLink sl;
		sl.timestamp = status_msg.timestamp;
		sl.name = l.name;
		sl.port = l.port;
		sl.baudrate = l.baud;
		sl.link_index = static_cast<uint8_t>(i);
		sl.link_count = count;

		bool online = false;
		auto it = router_stats.find(l.name);

		if (it != router_stats.end()) {
			const auto &ep = it->second;
			sl.rx_rate = ep.rx_rate;
			sl.tx_rate = ep.tx_rate;
			sl.rx_loss = ep.rx_loss_pct;
			sl.rx_bytes = ep.rx_bytes;
			sl.tx_bytes = ep.tx_bytes;
			online = ep.online;
		}

		UartHardwareStats hw = serial_stats.uart_stats(l.port);

		if (hw.valid) {
			sl.rx_errors = hw.rx_errors;
		}

		int &hold = hold_ticks_[l.name];
		uint8_t current_status = LINK_STATUS_DISCONNECTED;

		if (online) {
			hold = kHoldTicks;
			current_status = LINK_STATUS_CONNECTED;

		} else if (hold > 0) {
			--hold;
			current_status = LINK_STATUS_CONNECTED;
		}

		sl.status = current_status;

		// Detect state transitions
		if (last_status_.find(l.name) == last_status_.end()) {
			last_status_[l.name] = current_status;

			if (current_status == LINK_STATUS_CONNECTED) {
				out_transition_logs.push_back("LINKS: " + l.name + " ONLINE (" + l.port + ")");
			}

		} else if (current_status != last_status_[l.name]) {
			if (current_status == LINK_STATUS_CONNECTED) {
				out_transition_logs.push_back("LINKS: " + l.name + " ONLINE (" + l.port + ")");

			} else {
				out_transition_logs.push_back("LINKS: " + l.name + " OFFLINE");
			}

			last_status_[l.name] = current_status;
		}

		status_msg.links.push_back(sl);
	}

	return status_msg;
}

} // namespace cc
