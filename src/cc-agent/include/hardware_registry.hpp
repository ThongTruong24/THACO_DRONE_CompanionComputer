#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "nlohmann/json.hpp"

namespace cc {

struct UartHardwareStats {
    uint32_t rx_bytes{0};       ///< Delta RX bytes since last call (from TIOCGICOUNT ic.rx)
    uint32_t tx_bytes{0};       ///< Delta TX bytes since last call (from TIOCGICOUNT ic.tx)
    uint32_t rx_errors{0};      ///< Delta total RX errors (frame + overrun + parity + brk)
    uint32_t tx_errors{0};      ///< Reserved: not available via TIOCGICOUNT, always 0
    uint32_t frame_errors{0};   ///< Delta framing errors (stop-bit missing = baud rate mismatch)
    uint32_t overrun_errors{0}; ///< Delta hardware FIFO overrun errors
    uint32_t buf_overrun{0};    ///< Delta kernel receive-buffer overflow (data dropped by kernel)
    bool valid{false};
};

/// Raw snapshot of TIOCGICOUNT cumulative counters per device.
/// Used to compute deltas between successive calls to get_uart_stats().
struct UartCounterSnapshot {
    uint32_t rx{0};
    uint32_t tx{0};
    uint32_t frame{0};
    uint32_t overrun{0};
    uint32_t parity{0};
    uint32_t brk{0};
    uint32_t buf_overrun{0};
    bool initialized{false};
};

struct SerialDeviceInfo {
    std::string device_path;
    std::string description;
    std::string driver;
    bool is_available{true};
    std::string reserved_by;
};

/**
 * @brief Consolidated Hardware Registry adhering to Single Responsibility Principle.
 * Owns physical device enumeration, Kernel TIOCGICOUNT stats, and mutual-exclusion port reservations.
 * Fully subsumes legacy hardware-manager container logic.
 *
 * IMPORTANT: get_uart_stats() returns DELTA values since the previous call, not cumulative
 * lifetime counters. This ensures correct 1Hz rate reporting to GCS.
 */
class HardwareRegistry {
public:
    HardwareRegistry();
    ~HardwareRegistry() = default;

    // Device scanning & Kernel stats
    std::vector<SerialDeviceInfo> scan_devices();

    /**
     * @brief Read UART error counters for a device as DELTAS since last call.
     *
     * On first call, establishes a baseline from the current kernel cumulative counters.
     * On subsequent calls, returns the difference (delta) since the previous call.
     * This prevents 1.1M+ cumulative frame errors from being reported as instantaneous errors.
     *
     * @param device_path  Absolute path to UART device (e.g. "/dev/ttyAMA4")
     * @return UartHardwareStats with delta values; .valid = false if device inaccessible.
     */
    UartHardwareStats get_uart_stats(const std::string& device_path);
    bool is_valid_character_device(const std::string& device_path);

    // Mutual-exclusion port reservations
    bool reserve_port(const std::string& port, const std::string& consumer);
    bool release_port(const std::string& port);
    std::string get_port_consumer(const std::string& port);

    // Handler for legacy hardware-manager JSON commands (LIST_PORTS, VALIDATE_PORT, RESERVE_PORT, etc.)
    nlohmann::json process_json_command(const nlohmann::json& req);

private:
    std::mutex state_mutex_;
    std::unordered_map<std::string, std::string> port_reservations_;
    /// Per-device baseline snapshots for delta-error tracking.
    /// Protected by state_mutex_.
    std::unordered_map<std::string, UartCounterSnapshot> uart_snapshots_;
};

} // namespace cc
