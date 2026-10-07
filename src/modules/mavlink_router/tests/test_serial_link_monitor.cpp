#include <gtest/gtest.h>
#include "mavlink_router/serial_link_monitor.hpp"

using namespace cc;

class FakeSerialStats : public ISerialStats
{
public:
	UartHardwareStats uart_stats(const std::string &) override
	{
		UartHardwareStats s;
		s.valid = true;
		s.rx_errors = 3;
		return s;
	}

	uint32_t line_baud(const std::string &) override
	{
		return 921600;
	}
};

TEST(SerialLinkMonitorTest, HoldsConnectedFor3TicksAndLogsTransitions)
{
	SerialLinkMonitor monitor;
	FakeSerialStats fake_serial;
	Links links = {{"Primary", "/dev/ttyAMA4", 921600}};
	std::vector<std::string> avail_ports = {"/dev/ttyAMA4"};

	std::map<std::string, RouterEndpointStat> stats;
	RouterEndpointStat ep;
	ep.online = true;
	ep.rx_rate = 120.0f;
	ep.tx_rate = 80.0f;
	ep.rx_bytes = 1000;
	ep.tx_bytes = 500;
	stats["Primary"] = ep;

	std::vector<std::string> logs;

	// Tick 1: Online
	auto s1 = monitor.tick(links, stats, fake_serial, avail_ports, logs);
	ASSERT_EQ(s1.links.size(), 1u);
	EXPECT_EQ(s1.links[0].status, LINK_STATUS_CONNECTED);
	ASSERT_EQ(logs.size(), 1u);
	EXPECT_EQ(logs[0], "LINKS: Primary ONLINE (/dev/ttyAMA4)");

	// Tick 2: Offline - hold 1
	logs.clear();
	stats["Primary"].online = false;
	auto s2 = monitor.tick(links, stats, fake_serial, avail_ports, logs);
	EXPECT_EQ(s2.links[0].status, LINK_STATUS_CONNECTED);
	EXPECT_TRUE(logs.empty());

	// Tick 3: Offline - hold 2
	logs.clear();
	auto s3 = monitor.tick(links, stats, fake_serial, avail_ports, logs);
	EXPECT_EQ(s3.links[0].status, LINK_STATUS_CONNECTED);
	EXPECT_TRUE(logs.empty());

	// Tick 4: Offline - hold 3 (last tick of connected)
	logs.clear();
	auto s4 = monitor.tick(links, stats, fake_serial, avail_ports, logs);
	EXPECT_EQ(s4.links[0].status, LINK_STATUS_CONNECTED);
	EXPECT_TRUE(logs.empty());

	// Tick 5: Offline - hold expired -> DISCONNECTED
	logs.clear();
	auto s5 = monitor.tick(links, stats, fake_serial, avail_ports, logs);
	EXPECT_EQ(s5.links[0].status, LINK_STATUS_DISCONNECTED);
	ASSERT_EQ(logs.size(), 1u);
	EXPECT_EQ(logs[0], "LINKS: Primary OFFLINE");
}

TEST(SerialLinkMonitorTest, PopulatesStatusFields)
{
	SerialLinkMonitor monitor;
	FakeSerialStats fake_serial;
	Links links = {
		{"LinkA", "/dev/ttyUSB0", 115200},
		{"LinkB", "/dev/ttyUSB1", 57600}
	};
	std::vector<std::string> avail_ports = {"/dev/ttyUSB0", "/dev/ttyUSB1"};

	std::map<std::string, RouterEndpointStat> stats;
	stats["LinkA"] = {10.0f, 20.0f, 100, 200, 0.5f, true};

	std::vector<std::string> logs;
	auto res = monitor.tick(links, stats, fake_serial, avail_ports, logs);

	EXPECT_EQ(res.available_ports.size(), 2u);
	ASSERT_EQ(res.links.size(), 2u);

	EXPECT_EQ(res.links[0].name, "LinkA");
	EXPECT_EQ(res.links[0].link_index, 0);
	EXPECT_EQ(res.links[0].link_count, 2);
	EXPECT_EQ(res.links[0].rx_errors, 3u);
	EXPECT_FLOAT_EQ(res.links[0].rx_loss, 0.5f);

	EXPECT_EQ(res.links[1].name, "LinkB");
	EXPECT_EQ(res.links[1].link_index, 1);
	EXPECT_EQ(res.links[1].status, LINK_STATUS_DISCONNECTED);
}
