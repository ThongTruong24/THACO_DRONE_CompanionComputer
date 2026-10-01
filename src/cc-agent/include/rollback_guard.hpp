#pragma once

#include "config_engine.hpp"
#include <chrono>
#include <functional>

namespace cc {

enum class GuardState {
    IDLE,
    TRIAL,
    ROLLBACK_IN_PROGRESS
};

class RollbackGuard {
public:
    RollbackGuard(ConfigEngine& engine);

    void record_fc_heartbeat(size_t packet_size);
    bool start_trial(const TelemetryConfig& new_cfg);
    void tick_1hz();

    bool is_fc_online() const;
    float get_fc_bitrate_kbps() const { return fc_bitrate_kbps_; }
    uint32_t get_fc_rx_bytes() const { return fc_rx_bytes_; }
    uint8_t get_fc_status() const;
    const TelemetryConfig& get_current_config() const { return current_cfg_; }
    void update_config(const TelemetryConfig& cfg) { current_cfg_ = cfg; }
    void set_current_config(const TelemetryConfig& cfg) { current_cfg_ = cfg; }

    void set_log_callback(std::function<void(const std::string&, uint8_t)> cb) {
        log_callback_ = cb;
    }

private:
    ConfigEngine& engine_;
    GuardState state_ = GuardState::IDLE;

    TelemetryConfig current_cfg_;
    TelemetryConfig backup_cfg_;

    std::chrono::steady_clock::time_point last_fc_time_;
    std::chrono::steady_clock::time_point trial_start_time_;

    uint32_t fc_rx_bytes_ = 0;
    uint32_t fc_rx_bytes_last_ = 0;
    float fc_bitrate_kbps_ = 0.0f;

    std::function<void(const std::string&, uint8_t)> log_callback_;
};

} // namespace cc
