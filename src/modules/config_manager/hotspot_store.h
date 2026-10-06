#pragma once

#include <mutex>
#include <string>
#include <nlohmann/json.hpp>

namespace cc
{

struct HotspotStorePaths {
	std::string active_file = "/run/drone/hotspot.json";
	std::string wifi_file = "config/wifi.json";
};

bool is_valid_ssid(const std::string &s);
bool is_valid_wpa_passphrase(const std::string &s);

class HotspotStore
{
public:
	explicit HotspotStore(const HotspotStorePaths &paths);

	nlohmann::json effective() const;
	std::string effective_ssid() const;
	std::string effective_password() const;
	bool effective_valid() const;

	bool apply(const std::string &ssid, const std::string &password, std::string &err);
	bool save_default(std::string &err);
	bool restore_default(std::string &err);

	const HotspotStorePaths &paths() const { return _paths; }

private:
	HotspotStorePaths _paths;
	mutable std::mutex _mu;
};

} // namespace cc
