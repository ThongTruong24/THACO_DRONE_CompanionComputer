#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <link_config.hpp>
#include "mavlink_router/router_config_generator.hpp"
#include "mavlink_router/router_stats_parser.hpp"

namespace cc
{

class MavlinkRouter
{
public:
	explicit MavlinkRouter(std::string routerd_bin = "mavlink-routerd",
			       std::string config_path = "/run/drone/mavlink-router.conf");
	~MavlinkRouter();

	bool start(const std::string &active_links_path = "/run/drone/links.json",
		   const std::string &default_links_path = "/app/config/links.json");
	void stop();

	bool apply_links(const Links &links, std::string &err_msg);

	RouterConfig get_current_config() const;
	std::map<std::string, RouterEndpointStat> get_stats() const;

	bool is_running() const;
	bool is_child_alive() const;

private:
	static bool is_char_device(const std::string &port);
	std::vector<bool> check_ports_available(const Links &links) const;

	bool write_config_file_locked();
	bool spawn_child_locked();
	void kill_child_locked();

	void run_stats_reader();
	void run_hotplug_monitor();

	std::string routerd_bin_;
	std::string config_path_;

	mutable std::mutex config_mutex_;
	RouterConfig current_config_;
	pid_t child_pid_{-1};
	int pipe_read_fd_{-1};

	std::atomic<bool> running_{false};
	std::atomic<bool> stopped_{false};

	RouterStatsParser stats_parser_;

	std::condition_variable stop_cv_;
	std::thread stats_thread_;
	std::thread hotplug_thread_;
};

} // namespace cc
