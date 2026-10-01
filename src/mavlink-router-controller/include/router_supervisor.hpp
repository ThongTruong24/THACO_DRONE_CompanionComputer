#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <sys/types.h>

namespace drone::router {

struct RouterConfig {
    std::string fc_port{"/dev/ttyAMA4"};
    uint32_t fc_baud{921600};
    std::string siyi_port{"/dev/ttyAMA0"};
    uint32_t siyi_baud{115200};
    bool siyi_enabled{true};
    std::string gcs_ip{""};
    uint32_t tcp_port{5760};
};

class RouterSupervisor {
public:
    explicit RouterSupervisor(const std::string& socket_path = "/run/drone/router.sock",
                             const std::string& hw_socket_path = "/run/drone/hw_manager.sock",
                             const std::string& config_path = "/etc/mavlink-router/main.conf");
    ~RouterSupervisor();

    bool start();
    void stop();

    bool apply_config(const RouterConfig& cfg, std::string& err_msg);
    RouterConfig get_current_config() const;
    bool is_router_running() const;

    // Pure config generation helper (public for unit testing)
    static std::string generate_config_content(const RouterConfig& cfg, bool fc_valid, bool siyi_valid);
    bool generate_config_file(const RouterConfig& cfg);

    void set_config_path(const std::string& path) { config_path_ = path; }
    const std::string& get_config_path() const { return config_path_; }

    std::string process_json_command(const std::string& request_str);

private:
    void run_accept_loop();
    void handle_client(int client_fd);
    void run_hotplug_loop();

    bool validate_port_with_hw_manager(const std::string& port);
    bool restart_mavlink_routerd_process();

    std::string socket_path_;
    std::string hw_socket_path_;
    std::string config_path_;
    int server_fd_{-1};
    std::atomic<bool> running_{false};
    std::thread server_thread_;
    std::thread hotplug_thread_;

    mutable std::mutex config_mutex_;
    RouterConfig current_config_;
    pid_t routerd_pid_{-1};
};

} // namespace drone::router
