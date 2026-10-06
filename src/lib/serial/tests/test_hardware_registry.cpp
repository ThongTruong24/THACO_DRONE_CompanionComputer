#include <gtest/gtest.h>
#include "serial/hardware_registry.hpp"

using namespace cc;

class HardwareRegistryTest : public ::testing::Test
{
protected:
	HardwareRegistry reg;
};

TEST_F(HardwareRegistryTest, DetectCharacterDevice)
{
	EXPECT_TRUE(reg.is_valid_character_device("/dev/null"));
	EXPECT_TRUE(reg.is_valid_character_device("/dev/zero"));
	EXPECT_FALSE(reg.is_valid_character_device("/etc/passwd"));
	EXPECT_FALSE(reg.is_valid_character_device("/nonexistent/device/12345"));
	EXPECT_FALSE(reg.is_valid_character_device(""));
}

TEST_F(HardwareRegistryTest, ScanDevicesExecutesWithoutCrash)
{
	EXPECT_NO_THROW({
		auto ports = reg.scan_devices();
		(void)ports;
	});
}

TEST_F(HardwareRegistryTest, UartStatsInaccessibleDeviceReturnsInvalid)
{
	auto stats = reg.get_uart_stats("/dev/nonexistent_uart_99999");
	EXPECT_FALSE(stats.valid);
	EXPECT_EQ(stats.rx_bytes, 0u);
	EXPECT_EQ(stats.frame_errors, 0u);
	EXPECT_EQ(stats.buf_overrun, 0u);
}

TEST_F(HardwareRegistryTest, BaudFromTermiosSpeed)
{
	EXPECT_EQ(baud_from_termios_speed(B9600), 9600u);
	EXPECT_EQ(baud_from_termios_speed(B115200), 115200u);
	EXPECT_EQ(baud_from_termios_speed(B921600), 921600u);
	EXPECT_EQ(baud_from_termios_speed(B0), 0u);
}
