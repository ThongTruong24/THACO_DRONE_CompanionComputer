#include "rollback_guard.hpp"
#include <iostream>

namespace cc {

RollbackGuard::RollbackGuard(ConfigEngine& engine)
    : engine_(engine) {
    current_cfg_ = engine_.load_telemetry_config();
    backup_cfg_ = current_cfg_;
}

void RollbackGuard::record_fc_heartbeat(size_t packet_size) {
    last_fc_time_ = std::chrono::steady_clock::now();
    fc_rx_bytes_ += static_cast<uint32_t>(packet_size);
}

bool RollbackGuard::is_fc_online() const {
    if (last_fc_time_.time_since_epoch().count() == 0) return false;
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - last_fc_time_).count();
    return (elapsed < 3);
}

uint8_t RollbackGuard::get_fc_status() const {
    return is_fc_online() ? 2 : 0; // 2=ONLINE, 0=DISCONNECTED
}

bool RollbackGuard::start_trial(const TelemetryConfig& new_cfg) {
    backup_cfg_ = current_cfg_;
    current_cfg_ = new_cfg;

    if (!engine_.save_telemetry_config(new_cfg)) {
        if (log_callback_) {
            log_callback_("FC: ERROR saving config to .env", 3);
        }
        return false;
    }

    if (log_callback_) {
        log_callback_("LINKS: Applied Router config FC=" + new_cfg.fc_port + "@" + std::to_string(new_cfg.fc_baud) + ", SIYI=" + new_cfg.siyi_port + "@" + std::to_string(new_cfg.siyi_baud), 6);
    }

    // Reload router in background
    engine_.restart_mavlink_router();

    // No auto-rollback: configuration changes remain applied as requested by user
    state_ = GuardState::IDLE;
    return true;
}

void RollbackGuard::tick_1hz() {
    // Bitrate calculation
    uint32_t diff_bytes = (fc_rx_bytes_ >= fc_rx_bytes_last_) ? (fc_rx_bytes_ - fc_rx_bytes_last_) : 0;
    fc_rx_bytes_last_ = fc_rx_bytes_;
    fc_bitrate_kbps_ = is_fc_online() ? static_cast<float>((diff_bytes * 8.0) / 1000.0) : 0.0f;

    // No forced auto-rollback: user confirmed changes are not dangerous, keep user's configured settings!
}

} // namespace cc
