#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cc
{

struct LinkConfig {
	std::string name;
	std::string port;
	uint32_t baud = 0;
	bool operator==(const LinkConfig &) const = default;
};

using Links = std::vector<LinkConfig>;
constexpr size_t kMaxLinks = 8;

bool is_standard_baud(uint32_t baud);
bool is_valid_link_name(const std::string &name);   // [A-Za-z0-9_-]{1,15}
bool is_valid_port_path(const std::string &port);   // /dev/tty[A-Za-z0-9]{1,10}
std::string validate_links(const Links &links);     // "" = OK
std::optional<Links> parse_links_json(const std::string &text);   // parsed AND validated
std::string links_to_json(const Links &links);
std::optional<Links> load_links_file(const std::string &path);
bool save_links_file(const std::string &path, const Links &links);  // validated + atomic

} // namespace cc
