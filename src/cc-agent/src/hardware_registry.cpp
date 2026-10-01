#include "hardware_registry.hpp"
#include <filesystem>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <algorithm>

namespace fs = std::filesystem;

namespace cc {

HardwareRegistry::HardwareRegistry() = default;

bool HardwareRegistry::is_valid_character_device(const std::string& path) {
    if (path.empty()) return false;
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISCHR(st.st_mode);
}

std::vector<SerialDeviceInfo> HardwareRegistry::scan_devices() {
    std::vector<SerialDeviceInfo> list;
    const std::vector<std::string> prefixes = {
        "/dev/ttyAMA", "/dev/ttyUSB", "/dev/ttyACM", "/dev/ttyS"
    };

    try {
        if (fs::exists("/dev")) {
            for (const auto& entry : fs::directory_iterator("/dev")) {
                std::string path = entry.path().string();
                bool matches = false;
                for (const auto& pre : prefixes) {
                    if (path.rfind(pre, 0) == 0) {
                        matches = true;
                        break;
                    }
                }
                if (!matches) continue;

                if (is_valid_character_device(path)) {
                    SerialDeviceInfo dev;
                    dev.device_path = path;
                    dev.is_available = true;

                    if (path == "/dev/ttyAMA10") {
                        continue; // Skip Raspberry Pi 5 internal RP1 debug UART
                    }
                    if (path == "/dev/ttyAMA4") {
                        dev.description = "Flight Controller (Cube Orange Plus UART4)";
                        dev.driver = "pl011";
                    } else if (path == "/dev/ttyAMA0") {
                        dev.description = "SIYI Air Unit Telemetry (UART0)";
                        dev.driver = "pl011";
                    } else if (path.find("ttyUSB") != std::string::npos) {
                        dev.description = "USB-to-Serial Converter";
                        dev.driver = "usbserial";
                    } else if (path.find("ttyACM") != std::string::npos) {
                        dev.description = "USB CDC ACM Device";
                        dev.driver = "cdc_acm";
                    } else {
                        dev.description = "Standard Serial Port";
                        dev.driver = "serial";
                    }

                    {
                        std::lock_guard<std::mutex> lock(state_mutex_);
                        auto it = port_reservations_.find(path);
                        if (it != port_reservations_.end()) {
                            dev.reserved_by = it->second;
                        }
                    }

                    list.push_back(dev);
                }
            }
        }
    } catch (...) {}

    std::sort(list.begin(), list.end(), [](const SerialDeviceInfo& a, const SerialDeviceInfo& b) {
        return a.device_path < b.device_path;
    });

    return list;
}

/**
 * Returns DELTA UART counters since the previous call for this device path.
 *
 * WHY DELTA: TIOCGICOUNT provides cumulative counters since kernel UART driver init.
 * Reporting raw cumulative values as fc_tx_err/fc_rx_loss produces misleading numbers
 * (e.g. ic.frame = 1,140,845 from boot-time baud-rate negotiation appears as live errors).
 *
 * HOW: On first call, the current raw snapshot is stored as a baseline — stats returned
 * are all zeroes (valid=true). On subsequent calls, the delta since baseline is returned.
 *
 * SEMANTIC NOTE on field mapping:
 *   frame_errors  = ic.frame  : framing errors on RECEIVE path (stop-bit not detected)
 *                               Indicates baud-rate mismatch between Pi and FC.
 *                               Should contribute to fc_rx_loss, NOT fc_tx_err.
 *   overrun_errors = ic.overrun: hardware FIFO overrun on RECEIVE path.
 *   buf_overrun    = ic.buf_overrun: kernel receive-buffer overflow (data dropped by OS).
 *   tx_errors is always 0 — TIOCGICOUNT does not track TX-side errors.
 */
UartHardwareStats HardwareRegistry::get_uart_stats(const std::string& device_path) {
    UartHardwareStats stats{};
    if (device_path.empty()) return stats;

    int fd = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK | O_NOCTTY);
    if (fd < 0) {
        return stats;
    }

    struct serial_icounter_struct ic{};
    if (::ioctl(fd, TIOCGICOUNT, &ic) != 0) {
        ::close(fd);
        return stats;
    }
    ::close(fd);

    // Convert raw unsigned to avoid signed-integer subtraction UB
    uint32_t cur_rx         = static_cast<uint32_t>(ic.rx);
    uint32_t cur_tx         = static_cast<uint32_t>(ic.tx);
    uint32_t cur_frame      = static_cast<uint32_t>(ic.frame);
    uint32_t cur_overrun    = static_cast<uint32_t>(ic.overrun);
    uint32_t cur_parity     = static_cast<uint32_t>(ic.parity);
    uint32_t cur_brk        = static_cast<uint32_t>(ic.brk);
    uint32_t cur_buf_ovrn   = static_cast<uint32_t>(ic.buf_overrun);

    std::lock_guard<std::mutex> lock(state_mutex_);
    auto& snap = uart_snapshots_[device_path];

    if (!snap.initialized) {
        // First call: establish baseline; return zeroed deltas (valid=true)
        snap.rx          = cur_rx;
        snap.tx          = cur_tx;
        snap.frame       = cur_frame;
        snap.overrun     = cur_overrun;
        snap.parity      = cur_parity;
        snap.brk         = cur_brk;
        snap.buf_overrun = cur_buf_ovrn;
        snap.initialized = true;
        stats.valid = true;
        return stats;
    }

    // Delta computation: wrap-around safe (unsigned subtraction).
    stats.rx_bytes       = cur_rx       - snap.rx;
    stats.tx_bytes       = cur_tx       - snap.tx;
    stats.frame_errors   = cur_frame    - snap.frame;
    stats.overrun_errors = cur_overrun  - snap.overrun;
    stats.buf_overrun    = cur_buf_ovrn - snap.buf_overrun;
    stats.rx_errors      = stats.frame_errors + stats.overrun_errors
                         + (cur_parity - snap.parity)
                         + (cur_brk    - snap.brk);
    stats.tx_errors      = 0; // Not available from TIOCGICOUNT
    stats.valid          = true;

    // Update baseline
    snap.rx          = cur_rx;
    snap.tx          = cur_tx;
    snap.frame       = cur_frame;
    snap.overrun     = cur_overrun;
    snap.parity      = cur_parity;
    snap.brk         = cur_brk;
    snap.buf_overrun = cur_buf_ovrn;

    return stats;
}

bool HardwareRegistry::reserve_port(const std::string& port, const std::string& consumer) {
    if (!is_valid_character_device(port)) return false;
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto it = port_reservations_.find(port);
    if (it != port_reservations_.end() && it->second != consumer) {
        return false; // Already reserved by someone else
    }
    port_reservations_[port] = consumer;
    return true;
}

bool HardwareRegistry::release_port(const std::string& port) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return port_reservations_.erase(port) > 0;
}

std::string HardwareRegistry::get_port_consumer(const std::string& port) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto it = port_reservations_.find(port);
    if (it != port_reservations_.end()) return it->second;
    return "";
}

nlohmann::json HardwareRegistry::process_json_command(const nlohmann::json& req) {
    nlohmann::json res;
    std::string cmd = req.value("cmd", "");
    res["id"] = req.value("id", "");

    if (cmd == "LIST_PORTS") {
        auto ports = scan_devices();
        nlohmann::json p_list = nlohmann::json::array();
        for (const auto& p : ports) {
            p_list.push_back({
                {"device_path", p.device_path},
                {"description", p.description},
                {"driver", p.driver},
                {"is_available", p.is_available},
                {"reserved_by", p.reserved_by}
            });
        }
        res["status"] = "OK";
        res["ports"] = p_list;

    } else if (cmd == "VALIDATE_PORT") {
        std::string port = req.value("port", "");
        bool valid = is_valid_character_device(port);
        res["status"] = "OK";
        res["port"] = port;
        res["valid"] = valid;
        res["is_busy"] = false; // Never report busy to avoid flapping

    } else if (cmd == "RESERVE_PORT") {
        std::string port = req.value("port", "");
        std::string consumer = req.value("consumer", "unknown");
        if (reserve_port(port, consumer)) {
            res["status"] = "OK";
            res["port"] = port;
            res["reserved_by"] = consumer;
        } else {
            res["status"] = "ERROR";
            res["error"] = "Cannot reserve port: " + port;
        }

    } else if (cmd == "RELEASE_PORT") {
        std::string port = req.value("port", "");
        release_port(port);
        res["status"] = "OK";
        res["port"] = port;

    } else if (cmd == "GET_UART_STATS") {
        std::string port = req.value("port", "");
        auto st = get_uart_stats(port);
        res["status"] = "OK";
        res["port"] = port;
        res["rx_bytes"]       = st.rx_bytes;
        res["tx_bytes"]       = st.tx_bytes;
        res["rx_errors"]      = st.rx_errors;
        res["frame_errors"]   = st.frame_errors;
        res["overrun_errors"] = st.overrun_errors;
        res["buf_overrun"]    = st.buf_overrun;
        res["valid"]          = st.valid;

    } else if (cmd == "HEALTH") {
        res["status"] = "HEALTHY";
        res["service"] = "cc-agent-hardware";

    } else {
        res["status"] = "ERROR";
        res["error"] = "Unknown command: " + cmd;
    }

    return res;
}

} // namespace cc
