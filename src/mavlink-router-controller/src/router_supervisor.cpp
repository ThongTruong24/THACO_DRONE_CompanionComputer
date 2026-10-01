#include "router_supervisor.hpp"
#include "nlohmann/json.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace drone::router {

RouterSupervisor::RouterSupervisor(const std::string& socket_path,
                                   const std::string& hw_socket_path,
                                   const std::string& config_path)
    : socket_path_(socket_path),
      hw_socket_path_(hw_socket_path),
      config_path_(config_path) {
    // Read environment variables as initial config defaults
    if (const char* p = std::getenv("DRONE_SERIAL_PORT")) current_config_.fc_port = p;
    if (const char* b = std::getenv("DRONE_BAUD_RATE")) {
        try { current_config_.fc_baud = std::stoul(b); } catch (...) {}
    }
    if (const char* sp = std::getenv("SIYI_SERIAL_PORT")) current_config_.siyi_port = sp;
    if (const char* sb = std::getenv("SIYI_BAUD")) {
        try { current_config_.siyi_baud = std::stoul(sb); } catch (...) {}
    }
    if (const char* gcs = std::getenv("GCS_IP")) current_config_.gcs_ip = gcs;
    if (const char* tp = std::getenv("TCP_SERVER_PORT")) {
        try { current_config_.tcp_port = std::stoul(tp); } catch (...) {}
    }
}

RouterSupervisor::~RouterSupervisor() {
    stop();
}

bool RouterSupervisor::start() {
    if (running_.load()) return true;

    // 1. Initial config generation & launch mavlink-routerd
    generate_config_file(current_config_);
    restart_mavlink_routerd_process();

    // 2. Setup UDS Server
    try {
        fs::path p(socket_path_);
        if (p.has_parent_path()) fs::create_directories(p.parent_path());
    } catch (...) {}

    unlink(socket_path_.c_str());

    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "[RouterSupervisor] Socket create failed: " << strerror(errno) << "\n";
        return false;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(server_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[RouterSupervisor] Bind failed on " << socket_path_ << ": " << strerror(errno) << "\n";
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    chmod(socket_path_.c_str(), 0666);

    if (listen(server_fd_, 16) < 0) {
        std::cerr << "[RouterSupervisor] Listen failed: " << strerror(errno) << "\n";
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    running_.store(true);
    server_thread_ = std::thread(&RouterSupervisor::run_accept_loop, this);
    hotplug_thread_ = std::thread(&RouterSupervisor::run_hotplug_loop, this);

    std::cout << "[RouterSupervisor] IPC listening on " << socket_path_ << "\n";
    return true;
}

void RouterSupervisor::stop() {
    if (!running_.exchange(false)) return;

    if (server_fd_ >= 0) {
        shutdown(server_fd_, SHUT_RDWR);
        close(server_fd_);
        server_fd_ = -1;
    }

    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    if (hotplug_thread_.joinable()) {
        hotplug_thread_.join();
    }

    // Stop child mavlink-routerd process
    if (routerd_pid_ > 0) {
        kill(routerd_pid_, SIGTERM);
        int status;
        waitpid(routerd_pid_, &status, 0);
        routerd_pid_ = -1;
    }

    unlink(socket_path_.c_str());
    std::cout << "[RouterSupervisor] Stopped cleanly.\n";
}

void RouterSupervisor::run_accept_loop() {
    while (running_.load()) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd < 0) {
            if (!running_.load()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        std::thread(&RouterSupervisor::handle_client, this, client_fd).detach();
    }
}

void RouterSupervisor::run_hotplug_loop() {
    std::string last_detected_fc;
    bool last_fc_valid = false;
    bool last_siyi_valid = false;

    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        last_fc_valid = validate_port_with_hw_manager(current_config_.fc_port);
        last_siyi_valid = current_config_.siyi_enabled && validate_port_with_hw_manager(current_config_.siyi_port);
        last_detected_fc = current_config_.fc_port;
    }

    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        if (!running_.load()) break;

        RouterConfig cfg;
        {
            std::lock_guard<std::mutex> lock(config_mutex_);
            cfg = current_config_;
        }

        std::string candidate_fc = cfg.fc_port;
        const std::vector<std::string> usb_candidates = {
            "/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyUSB0", "/dev/ttyUSB1"
        };

        bool current_fc_valid = validate_port_with_hw_manager(candidate_fc);
        if (!current_fc_valid) {
            for (const auto& usb_dev : usb_candidates) {
                if (validate_port_with_hw_manager(usb_dev)) {
                    candidate_fc = usb_dev;
                    current_fc_valid = true;
                    break;
                }
            }
        }

        bool current_siyi_valid = cfg.siyi_enabled && validate_port_with_hw_manager(cfg.siyi_port);

        if (!is_router_running()) {
            std::cout << "[RouterSupervisor] ⚠️ Watchdog: mavlink-routerd died. Auto-restarting...\n";
            generate_config_file(cfg);
            restart_mavlink_routerd_process();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        bool changed = (current_fc_valid != last_fc_valid) ||
                       (current_siyi_valid != last_siyi_valid) ||
                       (candidate_fc != last_detected_fc && current_fc_valid);

        if (changed) {
            std::cout << "[RouterSupervisor] ⚡ Hotplug event: FC port '" << candidate_fc 
                      << "' (valid=" << current_fc_valid << "), SIYI port '" << cfg.siyi_port 
                      << "' (valid=" << current_siyi_valid << "). Reloading config...\n";

            cfg.fc_port = candidate_fc;
            {
                std::lock_guard<std::mutex> lock(config_mutex_);
                current_config_ = cfg;
            }

            generate_config_file(cfg);
            restart_mavlink_routerd_process();

            last_fc_valid = current_fc_valid;
            last_siyi_valid = current_siyi_valid;
            last_detected_fc = candidate_fc;
        }
    }
}

void RouterSupervisor::handle_client(int client_fd) {
    char buf[4096];
    std::string accumulated;

    while (running_.load()) {
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        accumulated += buf;

        size_t newline_pos = accumulated.find('\n');
        if (newline_pos != std::string::npos || (accumulated.front() == '{' && accumulated.back() == '}')) {
            std::string req = (newline_pos != std::string::npos) ? accumulated.substr(0, newline_pos) : accumulated;
            if (newline_pos != std::string::npos) {
                accumulated = accumulated.substr(newline_pos + 1);
            } else {
                accumulated.clear();
            }

            if (!req.empty()) {
                std::string reply = process_json_command(req);
                reply += "\n";
                send(client_fd, reply.data(), reply.size(), 0);
            }
        }
    }
    close(client_fd);
}

std::string RouterSupervisor::process_json_command(const std::string& request_str) {
    json res;
    try {
        json req = json::parse(request_str);
        std::string cmd = req.value("cmd", "");
        std::string req_id = req.value("id", "");
        res["id"] = req_id;

        if (cmd == "APPLY_CONFIG") {
            RouterConfig cfg;
            {
                std::lock_guard<std::mutex> lock(config_mutex_);
                cfg = current_config_;
            }

            if (req.contains("fc_port")) cfg.fc_port = req["fc_port"].get<std::string>();
            if (req.contains("fc_baud")) cfg.fc_baud = req["fc_baud"].get<uint32_t>();
            if (req.contains("siyi_port")) cfg.siyi_port = req["siyi_port"].get<std::string>();
            if (req.contains("siyi_baud")) cfg.siyi_baud = req["siyi_baud"].get<uint32_t>();
            if (req.contains("gcs_ip")) cfg.gcs_ip = req["gcs_ip"].get<std::string>();
            if (req.contains("tcp_port")) cfg.tcp_port = req["tcp_port"].get<uint32_t>();

            std::string err_msg;
            if (apply_config(cfg, err_msg)) {
                res["status"] = "OK";
                res["message"] = "Router configuration applied and process reloaded";
            } else {
                res["status"] = "ERROR";
                res["error"] = err_msg;
            }

        } else if (cmd == "GET_STATUS" || cmd == "HEALTH") {
            std::lock_guard<std::mutex> lock(config_mutex_);
            res["status"] = "OK";
            res["router_running"] = is_router_running();
            res["fc_port"] = current_config_.fc_port;
            res["fc_baud"] = current_config_.fc_baud;
            res["siyi_port"] = current_config_.siyi_port;
            res["siyi_baud"] = current_config_.siyi_baud;
            res["tcp_port"] = current_config_.tcp_port;

        } else {
            res["status"] = "ERROR";
            res["error"] = "Unknown command: " + cmd;
        }

    } catch (const std::exception& e) {
        res["status"] = "ERROR";
        res["error"] = std::string("JSON parse error: ") + e.what();
    }

    return res.dump();
}

bool RouterSupervisor::validate_port_with_hw_manager(const std::string& port) {
    if (port.empty()) return false;
    struct stat st{};
    return (stat(port.c_str(), &st) == 0 && S_ISCHR(st.st_mode));
}

std::string RouterSupervisor::generate_config_content(const RouterConfig& cfg, bool fc_valid, bool siyi_valid) {
    std::ostringstream out;
    out << "[General]\n"
        << "ReportStats = true\n"
        << "MavlinkDialect = ardupilotmega\n"
        << "DebugLogLevel = info\n"
        << "TcpServerPort = " << cfg.tcp_port << "\n\n";

    // 1. Flight Controller UART (only if port is valid to avoid mavlink-routerd crash)
    if (fc_valid && !cfg.fc_port.empty()) {
        out << "# 1. Flight Controller UART\n"
            << "[UartEndpoint FlightController]\n"
            << "Device = " << cfg.fc_port << "\n"
            << "Baud = " << cfg.fc_baud << "\n"
            << "FlowControl = false\n\n";
    } else {
        out << "# 1. Flight Controller UART (Standby: Port " << cfg.fc_port << " not active)\n\n";
    }

    out << "# 2. QGC GCS UDP Server (all network interfaces)\n"
        << "[UdpEndpoint QGC_UDP_Server]\n"
        << "Mode = Server\n"
        << "Address = 0.0.0.0\n"
        << "Port = 14550\n\n";

    out << "# 3. Companion Linux OS Agent\n"
        << "[UdpEndpoint ConfigAgent]\n"
        << "Mode = Server\n"
        << "Address = 127.0.0.1\n"
        << "Port = 14600\n\n";

    out << "# 4. MAVROS Companion Channel\n"
        << "[UdpEndpoint MAVROS_Companion]\n"
        << "Mode = Server\n"
        << "Address = 127.0.0.1\n"
        << "Port = 14541\n\n";

    // 5. SIYI Air Unit Telemetry (only if valid and different from FC port)
    if (cfg.siyi_enabled && siyi_valid && !cfg.siyi_port.empty() && cfg.siyi_port != cfg.fc_port) {
        out << "# 5. SIYI Air Unit Telemetry\n"
            << "[UartEndpoint SIYI_Serial]\n"
            << "Device = " << cfg.siyi_port << "\n"
            << "Baud = " << cfg.siyi_baud << "\n"
            << "FlowControl = false\n\n";
    }

    // 6. Direct GCS Unicast Push (from GCS_IP or dynamic config)
    if (!cfg.gcs_ip.empty()) {
        out << "# 6. Direct GCS Unicast Push\n"
            << "[UdpEndpoint QGC_Direct]\n"
            << "Mode = Normal\n"
            << "Address = " << cfg.gcs_ip << "\n"
            << "Port = 14550\n\n";
    }

    return out.str();
}

bool RouterSupervisor::generate_config_file(const RouterConfig& cfg) {
    try {
        fs::path p(config_path_);
        if (p.has_parent_path()) fs::create_directories(p.parent_path());
    } catch (...) {}

    bool fc_valid = validate_port_with_hw_manager(cfg.fc_port);
    bool siyi_valid = validate_port_with_hw_manager(cfg.siyi_port);

    if (!fc_valid) {
        std::cout << "[RouterSupervisor] Notice: FC port '" << cfg.fc_port
                  << "' is not currently available. Running in UDP Standby mode.\n";
    }

    std::string content = generate_config_content(cfg, fc_valid, siyi_valid);

    std::ofstream out(config_path_);
    if (!out.is_open()) {
        std::cerr << "[RouterSupervisor] Cannot write config file: " << config_path_ << "\n";
        return false;
    }

    out << content;
    return true;
}

bool RouterSupervisor::restart_mavlink_routerd_process() {
    if (routerd_pid_ > 0) {
        kill(routerd_pid_, SIGTERM);
        int status;
        waitpid(routerd_pid_, &status, 0);
        routerd_pid_ = -1;
    }

    // Check if mavlink-routerd exists
    if (system("which mavlink-routerd > /dev/null 2>&1") != 0) {
        std::cout << "[RouterSupervisor] Notice: mavlink-routerd binary not found on host (test environment mode).\n";
        return true;
    }

    // Fork and exec mavlink-routerd
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "[RouterSupervisor] fork() failed: " << strerror(errno) << "\n";
        return false;
    }

    std::string tcp_str = std::to_string(current_config_.tcp_port);
    if (pid == 0) {
        // Child process
        char* args[] = {
            const_cast<char*>("mavlink-routerd"),
            const_cast<char*>("-t"),
            const_cast<char*>(tcp_str.c_str()),
            const_cast<char*>("-c"),
            const_cast<char*>(config_path_.c_str()),
            nullptr
        };
        execvp("mavlink-routerd", args);
        std::cerr << "[RouterSupervisor] execvp(mavlink-routerd) failed: " << strerror(errno) << "\n";
        _exit(1);
    }

    routerd_pid_ = pid;
    std::cout << "[RouterSupervisor] mavlink-routerd launched (PID: " << routerd_pid_ << ")\n";
    return true;
}

bool RouterSupervisor::apply_config(const RouterConfig& cfg, std::string& err_msg) {
    // 1. Generate config file (with standby check)
    if (!generate_config_file(cfg)) {
        err_msg = "Failed to write " + config_path_;
        return false;
    }

    // 2. Restart mavlink-routerd child process cleanly
    if (!restart_mavlink_routerd_process()) {
        err_msg = "Failed to restart mavlink-routerd process";
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        current_config_ = cfg;
    }

    return true;
}

RouterConfig RouterSupervisor::get_current_config() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return current_config_;
}

bool RouterSupervisor::is_router_running() const {
    if (routerd_pid_ <= 0) return false;
    int status = 0;
    pid_t res = waitpid(routerd_pid_, &status, WNOHANG);
    if (res == routerd_pid_) {
        const_cast<RouterSupervisor*>(this)->routerd_pid_ = -1;
        return false;
    }
    return (kill(routerd_pid_, 0) == 0);
}

} // namespace drone::router
