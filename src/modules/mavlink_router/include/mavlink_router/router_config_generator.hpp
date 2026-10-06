#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <string>
#include <vector>
#include <link_config.hpp>

namespace cc
{

struct RouterConfig {
	Links links;
	std::string gcs_ip;
	uint32_t tcp_port{5760};
};

class RouterConfigGenerator
{
public:
	static bool is_unicast_gcs_ip(const std::string &ip);

	static std::string generate_config_content(const RouterConfig &cfg,
			const std::vector<bool> &port_ok);
};

} // namespace cc
