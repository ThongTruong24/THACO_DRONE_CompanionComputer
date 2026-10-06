#include "link_config.hpp"
#include "atomic_file.hpp"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>

namespace cc
{

bool is_standard_baud(uint32_t b)
{
	static const std::set<uint32_t> ok = {9600, 19200, 38400, 57600, 115200, 230400, 460800,
					      500000, 576000, 921600, 1000000, 1500000, 2000000
					     };
	return ok.count(b) > 0;
}

static bool chars_ok(const std::string &s, size_t max, bool allow_dash)
{
	return !s.empty() && s.size() <= max && std::all_of(s.begin(), s.end(), [&](char c) {
		return std::isalnum(static_cast<unsigned char>(c)) || (allow_dash && (c == '_' || c == '-'));
	});
}

bool is_valid_link_name(const std::string &n)
{
	return chars_ok(n, 15, true);
}

bool is_valid_port_path(const std::string &p)
{
	static const std::string pre = "/dev/tty";
	return p.rfind(pre, 0) == 0 && chars_ok(p.substr(pre.size()), 10, false);
}

std::string validate_links(const Links &links)
{
	if (links.empty() || links.size() > kMaxLinks) {
		return "link count must be 1.." + std::to_string(kMaxLinks);
	}

	std::set<std::string> names, ports;

	for (const auto &l : links) {
		if (!is_valid_link_name(l.name)) { return "bad name: " + l.name; }

		if (!is_valid_port_path(l.port)) { return "bad port: " + l.port; }

		if (!is_standard_baud(l.baud)) { return "bad baud: " + std::to_string(l.baud); }

		if (!names.insert(l.name).second) { return "duplicate name: " + l.name; }

		if (!ports.insert(l.port).second) { return "duplicate port: " + l.port; }
	}

	return "";
}

std::optional<Links> parse_links_json(const std::string &text)
{
	try {
		auto j = nlohmann::json::parse(text);

		if (!j.contains("links") || !j["links"].is_array()) {
			return std::nullopt;
		}

		Links links;

		for (const auto &e : j["links"]) {
			links.push_back({
				e.at("name").get<std::string>(),
				e.at("port").get<std::string>(),
				e.at("baud").get<uint32_t>()
			});
		}

		if (!validate_links(links).empty()) {
			return std::nullopt;
		}

		return links;

	} catch (...) {
		return std::nullopt;
	}
}

std::string links_to_json(const Links &links)
{
	nlohmann::json arr = nlohmann::json::array();

	for (const auto &l : links) {
		arr.push_back({{"name", l.name}, {"port", l.port}, {"baud", l.baud}});
	}

	return nlohmann::json{{"links", arr}}.dump(2) + "\n";
}

std::optional<Links> load_links_file(const std::string &path)
{
	std::ifstream in(path);

	if (!in) { return std::nullopt; }

	std::stringstream ss;
	ss << in.rdbuf();
	return parse_links_json(ss.str());
}

bool save_links_file(const std::string &path, const Links &links)
{
	return validate_links(links).empty() && write_file_atomic(path, links_to_json(links));
}

} // namespace cc
