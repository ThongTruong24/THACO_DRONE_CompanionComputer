#include "serial/hardware_registry.hpp"
#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace cc
{

uint32_t baud_from_termios_speed(speed_t speed)
{
	switch (speed) {
	case B9600:    return 9600;

	case B19200:   return 19200;

	case B38400:   return 38400;

	case B57600:   return 57600;

	case B115200:  return 115200;

	case B230400:  return 230400;

	case B460800:  return 460800;

	case B500000:  return 500000;

	case B576000:  return 576000;

	case B921600:  return 921600;

	case B1000000: return 1000000;

	case B1152000: return 1152000;

	case B1500000: return 1500000;

	case B2000000: return 2000000;

	case B3000000: return 3000000;

	default:       return 0; // B0 / unknown
	}
}

HardwareRegistry::HardwareRegistry() = default;

uint32_t HardwareRegistry::get_uart_line_baud(const std::string &device_path)
{
	if (device_path.empty()) { return 0; }

	int fd = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK | O_NOCTTY);

	if (fd < 0) { return 0; }

	struct termios tio {};

	if (::tcgetattr(fd, &tio) != 0) {
		::close(fd);
		return 0;
	}

	::close(fd);
	return baud_from_termios_speed(cfgetospeed(&tio));
}

bool HardwareRegistry::is_valid_character_device(const std::string &path)
{
	if (path.empty()) { return false; }

	struct stat st {};

	if (::stat(path.c_str(), &st) != 0) {
		return false;
	}

	return S_ISCHR(st.st_mode);
}

std::vector<SerialDeviceInfo> HardwareRegistry::scan_devices()
{
	std::vector<SerialDeviceInfo> list;
	const std::vector<std::string> prefixes = {
		"/dev/ttyAMA", "/dev/ttyUSB", "/dev/ttyACM", "/dev/ttyS"
	};

	try {
		if (fs::exists("/dev")) {
			for (const auto &entry : fs::directory_iterator("/dev")) {
				std::string path = entry.path().string();
				bool matches = false;

				for (const auto &pre : prefixes) {
					if (path.rfind(pre, 0) == 0) {
						matches = true;
						break;
					}
				}

				if (!matches) { continue; }

				if (is_valid_character_device(path)) {
					SerialDeviceInfo dev;
					dev.device_path = path;
					dev.is_available = true;

					if (path == "/dev/ttyAMA10") {
						continue; // Skip Raspberry Pi 5 internal RP1 debug UART
					}

					if (path.find("ttyUSB") != std::string::npos) {
						dev.description = "USB-to-Serial Converter";
						dev.driver = "usbserial";

					} else if (path.find("ttyACM") != std::string::npos) {
						dev.description = "USB CDC ACM Device";
						dev.driver = "cdc_acm";

					} else if (path.find("ttyAMA") != std::string::npos) {
						dev.description = "PL011 UART";
						dev.driver = "pl011";

					} else {
						dev.description = "Standard Serial Port";
						dev.driver = "serial";
					}

					list.push_back(dev);
				}
			}
		}

	} catch (...) {}

	std::sort(list.begin(), list.end(), [](const SerialDeviceInfo & a, const SerialDeviceInfo & b) {
		return a.device_path < b.device_path;
	});

	return list;
}

UartHardwareStats HardwareRegistry::get_uart_stats(const std::string &device_path)
{
	UartHardwareStats stats{};

	if (device_path.empty()) { return stats; }

	int fd = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK | O_NOCTTY);

	if (fd < 0) {
		return stats;
	}

	struct serial_icounter_struct ic {};

	if (::ioctl(fd, TIOCGICOUNT, &ic) != 0) {
		::close(fd);
		return stats;
	}

	::close(fd);

	uint32_t cur_rx         = static_cast<uint32_t>(ic.rx);
	uint32_t cur_tx         = static_cast<uint32_t>(ic.tx);
	uint32_t cur_frame      = static_cast<uint32_t>(ic.frame);
	uint32_t cur_overrun    = static_cast<uint32_t>(ic.overrun);
	uint32_t cur_parity     = static_cast<uint32_t>(ic.parity);
	uint32_t cur_brk        = static_cast<uint32_t>(ic.brk);
	uint32_t cur_buf_ovrn   = static_cast<uint32_t>(ic.buf_overrun);

	std::lock_guard<std::mutex> lock(state_mutex_);
	auto &snap = uart_snapshots_[device_path];

	if (!snap.initialized) {
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

	stats.rx_bytes       = cur_rx       - snap.rx;
	stats.tx_bytes       = cur_tx       - snap.tx;
	stats.frame_errors   = cur_frame    - snap.frame;
	stats.overrun_errors = cur_overrun  - snap.overrun;
	stats.buf_overrun    = cur_buf_ovrn - snap.buf_overrun;
	stats.rx_errors      = stats.frame_errors + stats.overrun_errors
			       + (cur_parity - snap.parity)
			       + (cur_brk    - snap.brk);
	stats.tx_errors      = 0;
	stats.valid          = true;

	snap.rx          = cur_rx;
	snap.tx          = cur_tx;
	snap.frame       = cur_frame;
	snap.overrun     = cur_overrun;
	snap.parity      = cur_parity;
	snap.brk         = cur_brk;
	snap.buf_overrun = cur_buf_ovrn;

	return stats;
}

} // namespace cc
