#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "nlohmann/json.hpp"
#include "config_engine.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

namespace cc {

ConfigEngine::ConfigEngine(const AppPaths& paths)
    : paths_(paths)
{
    init_defaults();
    load_config_file();
}

void ConfigEngine::init_defaults() {
    active_config_ = {
        {"network.eth0.mode", "static"},
        {"network.eth0.address", "192.168.144.150/24"},
        {"network.eth0.gateway", ""},
        {"network.eth0.dns", "1.1.1.1, 8.8.8.8"},
        {"network.eth0.mtu", "1500"},
        {"network.wlan0.enabled", "true"},
        {"network.wlan0.ssid", ""},
        {"network.wlan0.password", ""},
        {"network.hotspot.enabled", "true"},
        {"network.hotspot.ssid", "AP_DRONE"},
        {"network.hotspot.password", "12345678"},
        {"network.hotspot.channel", "6"},
        {"telemetry.fc.port", "/dev/ttyAMA4"},
        {"telemetry.fc.baudrate", "921600"},
        {"telemetry.siyi.port", "/dev/ttyAMA0"},
        {"telemetry.siyi.baudrate", "115200"},
        {"camera.main.resolution", "1920x1080"},
        {"camera.main.framerate", "30"},
        {"camera.main.bitrate", "2000"},
        {"camera.main.codec", "h264"},
        {"camera.main.encoder", "v4l2m2m"},
        {"vision.model", "yolov8n"},
        {"vision.confidence", "0.5"},
        {"vision.input_source", "rgb"}
    };
}

void ConfigEngine::load_config_file() {
    if (!fs::exists(paths_.config_file)) return;
    std::ifstream in(paths_.config_file);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string k = line.substr(0, pos);
            std::string v = line.substr(pos + 1);
            k.erase(0, k.find_first_not_of(" 	"));
            k.erase(k.find_last_not_of(" 	") + 1);
            v.erase(0, v.find_first_not_of(" \t\""));
            v.erase(v.find_last_not_of(" \t\"") + 1);
            if (!k.empty()) active_config_[k] = v;
        }
    }
}

void ConfigEngine::save_config_file() {
    try {
        fs::path p(paths_.config_file);
        if (p.has_parent_path()) fs::create_directories(p.parent_path());
        std::ofstream out(paths_.config_file);
        for (const auto& [k, v] : active_config_) {
            out << k << ": \"" << v << "\"\n";
        }
    } catch (...) {}
}

TelemetryConfig ConfigEngine::load_telemetry_config() {
    TelemetryConfig cfg;
    std::ifstream in(paths_.env_file);
    if (!in.is_open()) {
        cfg.fc_port = active_config_["telemetry.fc.port"];
        cfg.fc_baud = std::stoul(active_config_["telemetry.fc.baudrate"]);
        cfg.siyi_port = active_config_["telemetry.siyi.port"];
        cfg.siyi_baud = std::stoul(active_config_["telemetry.siyi.baudrate"]);
        return cfg;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("FC_PORT=", 0) == 0) cfg.fc_port = line.substr(8);
        else if (line.rfind("FC_BAUDRATE=", 0) == 0) cfg.fc_baud = std::stoul(line.substr(12));
        else if (line.rfind("SIYI_PORT=", 0) == 0) cfg.siyi_port = line.substr(10);
        else if (line.rfind("SIYI_BAUDRATE=", 0) == 0) cfg.siyi_baud = std::stoul(line.substr(14));
    }
    return cfg;
}

bool ConfigEngine::save_telemetry_config(const TelemetryConfig& config) {
    std::vector<std::string> lines;
    std::ifstream in(paths_.env_file);
    bool has_fc_p = false, has_fc_b = false, has_siyi_p = false, has_siyi_b = false;

    if (in.is_open()) {
        std::string line;
        while (std::getline(in, line)) {
            if (line.rfind("FC_PORT=", 0) == 0) {
                lines.push_back("FC_PORT=" + config.fc_port);
                has_fc_p = true;
            } else if (line.rfind("FC_BAUDRATE=", 0) == 0) {
                lines.push_back("FC_BAUDRATE=" + std::to_string(config.fc_baud));
                has_fc_b = true;
            } else if (line.rfind("SIYI_PORT=", 0) == 0) {
                lines.push_back("SIYI_PORT=" + config.siyi_port);
                has_siyi_p = true;
            } else if (line.rfind("SIYI_BAUDRATE=", 0) == 0) {
                lines.push_back("SIYI_BAUDRATE=" + std::to_string(config.siyi_baud));
                has_siyi_b = true;
            } else {
                lines.push_back(line);
            }
        }
        in.close();
    }

    if (!has_fc_p) lines.push_back("FC_PORT=" + config.fc_port);
    if (!has_fc_b) lines.push_back("FC_BAUDRATE=" + std::to_string(config.fc_baud));
    if (!has_siyi_p) lines.push_back("SIYI_PORT=" + config.siyi_port);
    if (!has_siyi_b) lines.push_back("SIYI_BAUDRATE=" + std::to_string(config.siyi_baud));

    std::ofstream out(paths_.env_file);
    if (!out.is_open()) return false;
    for (const auto& l : lines) out << l << "\n";
    return true;
}

bool ConfigEngine::restart_mavlink_router() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock >= 0) {
        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, paths_.router_sock.c_str(), sizeof(addr.sun_path) - 1);
        if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            auto cfg = load_telemetry_config();
            nlohmann::json req = {
                {"cmd", "APPLY_CONFIG"},
                {"fc_port", cfg.fc_port},
                {"fc_baud", cfg.fc_baud},
                {"siyi_port", cfg.siyi_port},
                {"siyi_baud", cfg.siyi_baud}
            };
            std::string payload = req.dump() + "\n";
            send(sock, payload.data(), payload.size(), 0);

            char buf[2048];
            ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
            close(sock);
            if (n > 0) {
                buf[n] = '\0';
                try {
                    auto resp = nlohmann::json::parse(buf);
                    if (resp.value("status", "") == "OK") {
                        if (log_cb_) log_cb_("SYS: Router reloaded via " + paths_.router_sock, 6);
                        return true;
                    }
                } catch (...) {}
            }
        } else {
            close(sock);
        }
    }

    // Fallback: systemctl restart
    int rc = std::system("systemctl restart mavlink-router.service 2>/dev/null");
    return (rc == 0);
}

TxResult ConfigEngine::begin(uint32_t timeout_s) {
    if (tx_state_ != TxState::IDLE) {
        return {false, active_tid_, 1, 0, 1001, "Transaction already in progress"};
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(100000, 999999);
    active_tid_ = dist(gen);
    timeout_s_ = timeout_s;
    backup_config_ = active_config_;
    staged_config_.clear();
    tx_state_ = TxState::STAGING;
    return {true, active_tid_, 0, 0, 0, "OK"};
}

TxResult ConfigEngine::set(uint32_t tid, const std::string& key, const std::string& val) {
    if (tx_state_ != TxState::STAGING || tid != active_tid_) {
        return {false, tid, 1, 0, 1002, "Invalid TID or transaction state"};
    }
    staged_config_[key] = val;
    return {true, tid, 0, determine_apply_type(key), 0, "Staged"};
}

std::pair<bool, std::string> ConfigEngine::get(const std::string& key) {
    if (tx_state_ == TxState::STAGING) {
        auto sit = staged_config_.find(key);
        if (sit != staged_config_.end()) return {true, sit->second};
    }
    auto it = active_config_.find(key);
    if (it != active_config_.end()) return {true, it->second};
    return {false, ""};
}

uint8_t ConfigEngine::determine_apply_type(const std::string& key) {
    if (key.rfind("network.", 0) == 0) return 2; // NETWORK_RESTART
    if (key.rfind("camera.", 0) == 0) return 1;  // SERVICE_RESTART
    if (key.rfind("rpi.overlay", 0) == 0) return 3; // REBOOT
    return 0; // HOT
}

TxResult ConfigEngine::apply(uint32_t tid) {
    if (tx_state_ != TxState::STAGING || tid != active_tid_) {
        return {false, tid, 1, 0, 1003, "Invalid TID or state for apply"};
    }

    for (const auto& [k, v] : staged_config_) {
        active_config_[k] = v;
    }
    save_config_file();

    // Check if telemetry link changed
    if (staged_config_.count("telemetry.fc.port") || staged_config_.count("telemetry.fc.baudrate") ||
        staged_config_.count("telemetry.siyi.port") || staged_config_.count("telemetry.siyi.baudrate")) {
        TelemetryConfig tc;
        tc.fc_port = active_config_["telemetry.fc.port"];
        tc.fc_baud = std::stoul(active_config_["telemetry.fc.baudrate"]);
        tc.siyi_port = active_config_["telemetry.siyi.port"];
        tc.siyi_baud = std::stoul(active_config_["telemetry.siyi.baudrate"]);
        save_telemetry_config(tc);
        restart_mavlink_router();
    }

    tx_state_ = TxState::WAIT_CONFIRM;
    arm_time_ = std::chrono::steady_clock::now();
    rollback_armed_ = true;

    if (log_cb_) {
        log_cb_("SYS: Configuration applied. Awaiting confirmation (" + std::to_string(timeout_s_) + "s)", 6);
    }

    return {true, tid, 0, 1, 0, "Applied. Awaiting confirm"};
}

TxResult ConfigEngine::confirm(uint32_t tid) {
    if (tx_state_ != TxState::WAIT_CONFIRM || tid != active_tid_) {
        return {false, tid, 1, 0, 1004, "Invalid TID or state for confirm"};
    }
    rollback_armed_ = false;
    tx_state_ = TxState::IDLE;
    active_tid_ = 0;
    staged_config_.clear();
    backup_config_.clear();
    if (log_cb_) log_cb_("SYS: Configuration confirmed successfully.", 6);
    return {true, tid, 0, 0, 0, "Confirmed"};
}

TxResult ConfigEngine::rollback(uint32_t tid) {
    (void)tid;
    active_config_ = backup_config_;
    save_config_file();
    rollback_armed_ = false;
    tx_state_ = TxState::IDLE;
    active_tid_ = 0;
    staged_config_.clear();
    backup_config_.clear();

    TelemetryConfig tc;
    tc.fc_port = active_config_["telemetry.fc.port"];
    tc.fc_baud = std::stoul(active_config_["telemetry.fc.baudrate"]);
    tc.siyi_port = active_config_["telemetry.siyi.port"];
    tc.siyi_baud = std::stoul(active_config_["telemetry.siyi.baudrate"]);
    save_telemetry_config(tc);
    restart_mavlink_router();

    if (log_cb_) log_cb_("SYS: [ROLLBACK] Reverted to safe configuration.", 4);
    return {true, 0, 0, 1, 0, "Rolled back"};
}

void ConfigEngine::tick_1hz() {
    if (rollback_armed_ && tx_state_ == TxState::WAIT_CONFIRM) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - arm_time_).count();
        if (elapsed >= timeout_s_) {
            rollback(active_tid_);
        }
    }
}

} // namespace cc
