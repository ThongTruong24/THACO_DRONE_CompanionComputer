#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <termios.h>
#include <unordered_map>
#include <vector>

namespace cc
{

/// Map a termios speed constant (e.g. B921600) to its numeric baud (921600).
/// Returns 0 for B0/unknown. Pure + unit-testable.
uint32_t baud_from_termios_speed(speed_t speed);

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
};

/**
 * @brief Consolidated Hardware Registry for physical device enumeration and Kernel TIOCGICOUNT stats.
 * get_uart_stats() returns DELTA values since the previous call.
 */
class HardwareRegistry
{
public:
	HardwareRegistry();
	~HardwareRegistry() = default;

	// Device scanning & Kernel stats
	std::vector<SerialDeviceInfo> scan_devices();

	/**
	 * @brief Read UART error counters for a device as DELTAS since last call.
	 *
	 * @param device_path  Absolute path to UART device (e.g. "/dev/ttyAMA4")
	 * @return UartHardwareStats with delta values; .valid = false if device inaccessible.
	 */
	UartHardwareStats get_uart_stats(const std::string &device_path);

	/// Read the UART's REAL configured line speed from the kernel (tcgetattr).
	uint32_t get_uart_line_baud(const std::string &device_path);

	bool is_valid_character_device(const std::string &device_path);

private:
	std::mutex state_mutex_;
	std::unordered_map<std::string, UartCounterSnapshot> uart_snapshots_;
};

} // namespace cc
