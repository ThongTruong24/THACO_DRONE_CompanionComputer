#include <gtest/gtest.h>
#include "mavlink_router/router_config_generator.hpp"

using namespace cc;

TEST(RouterConfigGeneratorTest, OnlyUnicastAccepted)
{
	EXPECT_TRUE(RouterConfigGenerator::is_unicast_gcs_ip("192.168.10.23"));
	EXPECT_TRUE(RouterConfigGenerator::is_unicast_gcs_ip("10.0.0.5"));
	EXPECT_TRUE(RouterConfigGenerator::is_unicast_gcs_ip("172.16.0.100"));

	for (const char *bad : {"192.168.10.255", "255.255.255.255", "224.1.1.1", "0.0.0.0", "drone.local", "", "not_an_ip"}) {
		EXPECT_FALSE(RouterConfigGenerator::is_unicast_gcs_ip(bad)) << bad;
	}
}

TEST(RouterConfigGeneratorTest, OneEndpointPerAvailableNamedLink)
{
	RouterConfig cfg;
	cfg.links = {
		{"FC", "/dev/ttyAMA4", 921600},
		{"Radio2", "/dev/ttyUSB0", 57600}
	};
	std::string out = RouterConfigGenerator::generate_config_content(cfg, {true, false});

	EXPECT_NE(out.find("[UartEndpoint FC]\nDevice = /dev/ttyAMA4\nBaud = 921600"), std::string::npos);
	EXPECT_EQ(out.find("[UartEndpoint Radio2]"), std::string::npos);
	EXPECT_NE(out.find("# Radio2: /dev/ttyUSB0 not present (standby)"), std::string::npos);

	EXPECT_NE(out.find("Port = 14550"), std::string::npos);
	EXPECT_NE(out.find("[UdpEndpoint ConfigAgent]\nMode = Server\nAddress = 127.0.0.1\nPort = 14600"), std::string::npos);
	EXPECT_EQ(out.find("14541"), std::string::npos);
}

TEST(RouterConfigGeneratorTest, StandbyWhenNoLinkAvailable)
{
	RouterConfig cfg;
	cfg.links = {{"FC", "/dev/ttyAMA4", 921600}};
	std::string out = RouterConfigGenerator::generate_config_content(cfg, {false});
	EXPECT_EQ(out.find("[UartEndpoint"), std::string::npos);
	EXPECT_NE(out.find("# FC: /dev/ttyAMA4 not present (standby)"), std::string::npos);
}

TEST(RouterConfigGeneratorTest, DuplicatePortSkipped)
{
	RouterConfig cfg;
	cfg.links = {
		{"Link1", "/dev/ttyUSB0", 115200},
		{"Link2", "/dev/ttyUSB0", 57600}
	};
	std::string out = RouterConfigGenerator::generate_config_content(cfg, {true, true});
	EXPECT_NE(out.find("[UartEndpoint Link1]"), std::string::npos);
	EXPECT_EQ(out.find("[UartEndpoint Link2]"), std::string::npos);
	EXPECT_NE(out.find("# Link2: duplicate port /dev/ttyUSB0 skipped"), std::string::npos);
}

TEST(RouterConfigGeneratorTest, QgcDirectOnlyForUnicast)
{
	RouterConfig cfg;
	cfg.gcs_ip = "192.168.1.50";
	std::string out = RouterConfigGenerator::generate_config_content(cfg, {});
	EXPECT_NE(out.find("[UdpEndpoint QGC_Direct]"), std::string::npos);
	EXPECT_NE(out.find("Address = 192.168.1.50"), std::string::npos);

	cfg.gcs_ip = "192.168.1.255"; // broadcast
	out = RouterConfigGenerator::generate_config_content(cfg, {});
	EXPECT_EQ(out.find("[UdpEndpoint QGC_Direct]"), std::string::npos);
}
