#include "hotspot_store.h"

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <atomic_file.hpp>

namespace cc
{

static bool is_printable(const std::string &s, size_t min_len, size_t max_len)
{
	if (s.size() < min_len || s.size() > max_len) {
		return false;
	}

	for (unsigned char c : s) {
		if (c < 0x20 || c > 0x7E) {
			return false;
		}
	}

	return true;
}

bool is_valid_ssid(const std::string &s)
{
	return is_printable(s, 1, 32);
}

bool is_valid_wpa_passphrase(const std::string &s)
{
	return is_printable(s, 8, 63);
}

static nlohmann::json read_json(const std::string &path)
{
	std::ifstream in(path);

	if (!in) {
		return nlohmann::json::object();
	}

	try {
		return nlohmann::json::parse(in);

	} catch (...) {
		return nlohmann::json::object();
	}
}

static bool valid_hotspot(const nlohmann::json &h)
{
	return h.is_object() &&
	       h.contains("ssid") && h["ssid"].is_string() && is_valid_ssid(h["ssid"].get<std::string>()) &&
	       h.contains("password") && h["password"].is_string() && is_valid_wpa_passphrase(h["password"].get<std::string>());
}

HotspotStore::HotspotStore(const HotspotStorePaths &paths)
	: _paths(paths)
{
}

nlohmann::json HotspotStore::effective() const
{
	auto a = read_json(_paths.active_file);

	if (valid_hotspot(a)) {
		return a;
	}

	auto d = read_json(_paths.wifi_file).value("hotspot", nlohmann::json::object());

	if (valid_hotspot(d)) {
		return d;
	}

	return nlohmann::json::object();
}

std::string HotspotStore::effective_ssid() const
{
	std::lock_guard<std::mutex> l(_mu);
	return effective().value("ssid", "");
}

std::string HotspotStore::effective_password() const
{
	std::lock_guard<std::mutex> l(_mu);
	return effective().value("password", "");
}

bool HotspotStore::effective_valid() const
{
	std::lock_guard<std::mutex> l(_mu);
	return !effective().empty();
}

bool HotspotStore::apply(const std::string &ssid, const std::string &password, std::string &err)
{
	std::lock_guard<std::mutex> l(_mu);
	const auto cur = effective();
	const std::string s = ssid.empty() ? cur.value("ssid", "") : ssid;
	const std::string p = password.empty() ? cur.value("password", "") : password;

	if (!is_valid_ssid(s)) {
		err = "invalid SSID";
		return false;
	}

	if (!is_valid_wpa_passphrase(p)) {
		err = "WPA2 passphrase must be 8..63 printable ASCII";
		return false;
	}

	nlohmann::json body = {{"ssid", s}, {"password", p}};

	if (write_file_atomic(_paths.active_file, body.dump(2) + "\n")) {
		return true;
	}

	err = "cannot write " + _paths.active_file;
	return false;
}

bool HotspotStore::save_default(std::string &err)
{
	std::lock_guard<std::mutex> l(_mu);
	auto a = read_json(_paths.active_file);

	if (!a.contains("password")) {
		return true; // nothing active -> nothing to save
	}

	auto w = read_json(_paths.wifi_file);
	w["hotspot"] = a;

	if (write_file_atomic(_paths.wifi_file, w.dump(2) + "\n")) {
		return true;
	}

	err = "cannot write " + _paths.wifi_file;
	return false;
}

bool HotspotStore::restore_default(std::string &err)
{
	std::lock_guard<std::mutex> l(_mu);

	if (std::remove(_paths.active_file.c_str()) == 0 || errno == ENOENT) {
		return true;
	}

	err = "cannot remove " + _paths.active_file;
	return false;
}

} // namespace cc
