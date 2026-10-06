#include "mavlink_router/mavlink_router.hpp"
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace cc
{

MavlinkRouter::MavlinkRouter(std::string routerd_bin, std::string config_path)
	: routerd_bin_(std::move(routerd_bin)),
	  config_path_(std::move(config_path))
{
}

MavlinkRouter::~MavlinkRouter()
{
	stop();
}

bool MavlinkRouter::is_char_device(const std::string &port)
{
	if (port.empty()) {
		return false;
	}

	struct stat st {};

	return (stat(port.c_str(), &st) == 0 && S_ISCHR(st.st_mode));
}

std::vector<bool> MavlinkRouter::check_ports_available(const Links &links) const
{
	std::vector<bool> ok;
	ok.reserve(links.size());

	for (const auto &l : links) {
		ok.push_back(is_char_device(l.port));
	}

	return ok;
}

bool MavlinkRouter::is_running() const
{
	return running_.load() && !stopped_.load();
}

bool MavlinkRouter::is_child_alive() const
{
	std::lock_guard<std::mutex> lock(config_mutex_);

	if (child_pid_ <= 0) {
		return false;
	}

	int status = 0;
	pid_t res = waitpid(child_pid_, &status, WNOHANG);
	return (res == 0);
}

RouterConfig MavlinkRouter::get_current_config() const
{
	std::lock_guard<std::mutex> lock(config_mutex_);
	return current_config_;
}

std::map<std::string, RouterEndpointStat> MavlinkRouter::get_stats() const
{
	return stats_parser_.get_endpoint_stats();
}

bool MavlinkRouter::start(const std::string &active_links_path,
			  const std::string &default_links_path)
{
	if (running_.load()) {
		return true;
	}

	stopped_.store(false);
	running_.store(true);

	// Load initial links: Active wins over default
	const char *env_active = std::getenv("ROUTER_LINKS_ACTIVE");
	const char *env_default = std::getenv("ROUTER_LINKS_DEFAULT");
	std::string p_active = (env_active && *env_active) ? env_active : active_links_path;
	std::string p_default = (env_default && *env_default) ? env_default : default_links_path;

	Links loaded_links;

	if (auto opt = load_links_file(p_active); opt.has_value()) {
		loaded_links = std::move(*opt);

	} else if (auto opt_def = load_links_file(p_default); opt_def.has_value()) {
		loaded_links = std::move(*opt_def);
	}

	std::string gcs;

	if (const char *env_gcs = std::getenv("GCS_IP"); env_gcs && *env_gcs) {
		if (RouterConfigGenerator::is_unicast_gcs_ip(env_gcs)) {
			gcs = env_gcs;

		} else {
			std::cerr << "[MavlinkRouter] Ignoring non-unicast GCS_IP: " << env_gcs << "\n";
		}
	}

	uint32_t tcp_port = 5760;

	if (const char *env_tcp = std::getenv("TCP_SERVER_PORT"); env_tcp && *env_tcp) {
		try {
			tcp_port = std::stoul(env_tcp);

		} catch (...) {}
	}

	{
		std::lock_guard<std::mutex> lock(config_mutex_);
		current_config_.links = std::move(loaded_links);
		current_config_.gcs_ip = std::move(gcs);
		current_config_.tcp_port = tcp_port;

		write_config_file_locked();
		spawn_child_locked();
	}

	stats_thread_ = std::thread(&MavlinkRouter::run_stats_reader, this);
	hotplug_thread_ = std::thread(&MavlinkRouter::run_hotplug_monitor, this);

	return true;
}

void MavlinkRouter::stop()
{
	bool expected = true;

	if (!running_.compare_exchange_strong(expected, false)) {
		return;
	}

	stopped_.store(true);
	stop_cv_.notify_all();

	{
		std::lock_guard<std::mutex> lock(config_mutex_);
		kill_child_locked();

		if (pipe_read_fd_ >= 0) {
			close(pipe_read_fd_);
			pipe_read_fd_ = -1;
		}
	}

	if (stats_thread_.joinable()) {
		stats_thread_.join();
	}

	if (hotplug_thread_.joinable()) {
		hotplug_thread_.join();
	}
}

bool MavlinkRouter::apply_links(const Links &links, std::string &err_msg)
{
	// Fix cdo.3: do not spawn child if already stopped
	if (stopped_.load() || !running_.load()) {
		err_msg = "Router is stopped";
		return false;
	}

	std::string val = validate_links(links);

	if (!val.empty()) {
		err_msg = val;
		return false;
	}

	std::lock_guard<std::mutex> lock(config_mutex_);

	if (stopped_.load() || !running_.load()) {
		err_msg = "Router is stopped";
		return false;
	}

	current_config_.links = links;

	if (!write_config_file_locked()) {
		err_msg = "Failed to write config file";
		return false;
	}

	kill_child_locked();

	if (!spawn_child_locked()) {
		err_msg = "Failed to restart router child process";
		return false;
	}

	return true;
}

bool MavlinkRouter::write_config_file_locked()
{
	try {
		fs::path p(config_path_);

		if (p.has_parent_path()) {
			fs::create_directories(p.parent_path());
		}

	} catch (...) {}

	std::vector<bool> port_ok = check_ports_available(current_config_.links);
	std::string content = RouterConfigGenerator::generate_config_content(current_config_, port_ok);

	std::ofstream out(config_path_);

	if (!out) {
		std::cerr << "[MavlinkRouter] Cannot open config file for write: " << config_path_ << "\n";
		return false;
	}

	out << content;
	return out.good();
}

bool MavlinkRouter::spawn_child_locked()
{
	int pipefd[2];

	if (pipe(pipefd) < 0) {
		std::cerr << "[MavlinkRouter] pipe() failed\n";
		return false;
	}

	pid_t pid = fork();

	if (pid < 0) {
		std::cerr << "[MavlinkRouter] fork() failed\n";
		close(pipefd[0]);
		close(pipefd[1]);
		return false;
	}

	if (pid == 0) {
		// Child
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);

		execlp(routerd_bin_.c_str(), routerd_bin_.c_str(), "-c", config_path_.c_str(), nullptr);
		_exit(127);
	}

	// Parent
	close(pipefd[1]);

	if (pipe_read_fd_ >= 0) {
		close(pipe_read_fd_);
	}

	pipe_read_fd_ = pipefd[0];
	child_pid_ = pid;
	return true;
}

void MavlinkRouter::kill_child_locked()
{
	if (child_pid_ <= 0) {
		return;
	}

	kill(child_pid_, SIGTERM);

	for (int i = 0; i < 20; ++i) {
		int status = 0;
		pid_t r = waitpid(child_pid_, &status, WNOHANG);

		if (r > 0 || (r < 0 && errno == ECHILD)) {
			child_pid_ = -1;
			return;
		}

		usleep(50000); // 50ms * 20 = 1.0s
	}

	kill(child_pid_, SIGKILL);
	int status = 0;
	waitpid(child_pid_, &status, 0);
	child_pid_ = -1;
}

void MavlinkRouter::run_stats_reader()
{
	char line_buf[1024];

	while (running_.load()) {
		int fd = -1;
		{
			std::lock_guard<std::mutex> lock(config_mutex_);
			fd = pipe_read_fd_;
		}

		if (fd < 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		FILE *stream = fdopen(dup(fd), "r");

		if (!stream) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		while (running_.load() && fgets(line_buf, sizeof(line_buf), stream)) {
			std::string line(line_buf);

			while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
				line.pop_back();
			}

			stats_parser_.feed_line(line);
		}

		fclose(stream);
	}
}

void MavlinkRouter::run_hotplug_monitor()
{
	std::unique_lock<std::mutex> lock(config_mutex_);
	std::vector<bool> last_ok = check_ports_available(current_config_.links);

	while (!stopped_.load() && running_.load()) {
		// Fix cdo.2: condvar wait_for interruptible by stop()
		if (stop_cv_.wait_for(lock, std::chrono::seconds(5), [this] { return stopped_.load(); })) {
			break;
		}

		// Check if child died
		bool child_alive = false;

		if (child_pid_ > 0) {
			int status = 0;
			pid_t r = waitpid(child_pid_, &status, WNOHANG);
			child_alive = (r == 0);
		}

		std::vector<bool> current_ok = check_ports_available(current_config_.links);
		bool ports_changed = (current_ok != last_ok);

		if (!child_alive || ports_changed) {
			std::cerr << "[MavlinkRouter] Hotplug change or child died, restarting...\n";
			write_config_file_locked();
			kill_child_locked();
			spawn_child_locked();
			last_ok = current_ok;
		}
	}
}

} // namespace cc
