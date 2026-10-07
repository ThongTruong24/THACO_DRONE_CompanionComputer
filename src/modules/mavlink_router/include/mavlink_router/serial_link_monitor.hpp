#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <link_config.hpp>
#include <serial/i_serial_stats.hpp>
#include <serial/link_stats.hpp>
#include "cc_msgs/msg/serial_link_status.hpp"

namespace cc
{

class SerialLinkMonitor
{
public:
	static constexpr int kHoldTicks = 3;

	SerialLinkMonitor() = default;

	cc_msgs::msg::SerialLinkStatus tick(
		const Links &active_links,
		const std::map<std::string, RouterEndpointStat> &router_stats,
		ISerialStats &serial_stats,
		const std::vector<std::string> &available_ports,
		std::vector<std::string> &out_transition_logs);

	void reset();

private:
	std::map<std::string, int> hold_ticks_;
	std::map<std::string, uint8_t> last_status_;
};

} // namespace cc
