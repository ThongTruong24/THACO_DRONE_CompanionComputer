#include "mavlink_router/router_config_generator.hpp"
#include <sstream>
#include <unordered_set>

namespace cc
{

bool RouterConfigGenerator::is_unicast_gcs_ip(const std::string &ip)
{
	in_addr a{};

	if (::inet_pton(AF_INET, ip.c_str(), &a) != 1) {
		return false;
	}

	const uint32_t h = ntohl(a.s_addr);
	// Reject 0.0.0.0, 255.255.255.255, multicast (224.0.0.0/4 -> high 4 bits == 0xE),
	// and subnet broadcast x.x.x.255 (/24 AP/LAN)
	return h != 0 && h != 0xFFFFFFFFu && (h >> 28) != 0xE && (h & 0xFFu) != 0xFFu;
}

std::string RouterConfigGenerator::generate_config_content(const RouterConfig &cfg,
		const std::vector<bool> &port_ok)
{
	std::ostringstream out;
	out << "[General]\n"
	    << "ReportStats = true\n"
	    << "MavlinkDialect = ardupilotmega\n"
	    << "DebugLogLevel = info\n"
	    << "TcpServerPort = " << cfg.tcp_port << "\n\n";

	std::unordered_set<std::string> seen_ports;

	for (size_t i = 0; i < cfg.links.size(); ++i) {
		const auto &l = cfg.links[i];

		if (l.port.empty()) {
			continue;
		}

		if (seen_ports.count(l.port)) {
			out << "# " << l.name << ": duplicate port " << l.port << " skipped\n\n";
			continue;
		}

		seen_ports.insert(l.port);

		bool ok = (i < port_ok.size()) ? port_ok[i] : false;

		if (ok) {
			out << "[UartEndpoint " << l.name << "]\n"
			    << "Device = " << l.port << "\n"
			    << "Baud = " << l.baud << "\n"
			    << "FlowControl = false\n\n";

		} else {
			out << "# " << l.name << ": " << l.port << " not present (standby)\n\n";
		}
	}

	out << "[UdpEndpoint QGC_UDP_Server]\n"
	    << "Mode = Server\n"
	    << "Address = 0.0.0.0\n"
	    << "Port = 14550\n\n";

	out << "[UdpEndpoint ConfigAgent]\n"
	    << "Mode = Server\n"
	    << "Address = 127.0.0.1\n"
	    << "Port = 14600\n\n";

	if (is_unicast_gcs_ip(cfg.gcs_ip)) {
		out << "[UdpEndpoint QGC_Direct]\n"
		    << "Mode = Normal\n"
		    << "Address = " << cfg.gcs_ip << "\n"
		    << "Port = 14550\n\n";
	}

	return out.str();
}

} // namespace cc
