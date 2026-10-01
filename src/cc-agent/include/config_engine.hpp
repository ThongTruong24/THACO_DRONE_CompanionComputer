#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <chrono>
#include <functional>
#include <utility>
#include "app_paths.hpp"

namespace cc {

struct TelemetryConfig {
    std::string fc_port = "/dev/ttyAMA4";
    uint32_t    fc_baud = 921600;
    std::string siyi_port = "/dev/ttyAMA0";
    uint32_t    siyi_baud = 115200;
};

struct TxResult {
    bool        success     = true;
    uint32_t    tid         = 0;
    uint8_t     result_code = 0; // 0=OK, 1=ERR, 2=REJECTED
    uint8_t     apply_type  = 0; // 0=HOT, 1=SERVICE_RESTART, 2=NETWORK_RESTART, 3=REBOOT
    uint16_t    error_code  = 0;
    std::string message;
};

class ConfigEngine {
public:
    explicit ConfigEngine(const AppPaths& paths = AppPaths::resolve());

    // Telemetry .env helpers (used by RollbackGuard & serial auto-switch)
    TelemetryConfig load_telemetry_config();
    bool save_telemetry_config(const TelemetryConfig& config);
    bool restart_mavlink_router();

    // Transactional configuration API (Native C++ replaces cc-configd)
    TxResult begin(uint32_t timeout_s = 30);
    TxResult set(uint32_t tid, const std::string& key, const std::string& val);
    std::pair<bool, std::string> get(const std::string& key);
    TxResult apply(uint32_t tid);
    TxResult confirm(uint32_t tid);
    TxResult rollback(uint32_t tid = 0);

    // Periodic tick for safety rollback timer (called at 1 Hz from main loop)
    void tick_1hz();

    void set_log_callback(std::function<void(const std::string&, uint8_t)> cb) {
        log_cb_ = std::move(cb);
    }

    const AppPaths& paths() const { return paths_; }

private:
    void init_defaults();
    void load_config_file();
    void save_config_file();

    // Backends
    bool apply_netplan_backend(const std::map<std::string, std::string>& changes, std::string& out_msg);
    bool apply_rpi_hardware_backend(const std::map<std::string, std::string>& changes, std::string& out_msg);
    bool apply_systemd_backend(const std::map<std::string, std::string>& changes, std::string& out_msg);

    uint8_t determine_apply_type(const std::string& key);

    AppPaths paths_;

    std::map<std::string, std::string> active_config_;
    std::map<std::string, std::string> backup_config_;
    std::map<std::string, std::string> staged_config_;

    enum class TxState { IDLE, STAGING, APPLYING, WAIT_CONFIRM, COMMITTED, ROLLBACK };
    TxState tx_state_ = TxState::IDLE;
    uint32_t active_tid_ = 0;
    uint32_t timeout_s_ = 30;
    std::chrono::steady_clock::time_point arm_time_;
    bool rollback_armed_ = false;

    std::function<void(const std::string&, uint8_t)> log_cb_;
};

} // namespace cc
