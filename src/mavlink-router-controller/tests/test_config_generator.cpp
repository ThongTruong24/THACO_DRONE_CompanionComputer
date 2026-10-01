#include <gtest/gtest.h>
#include "router_supervisor.hpp"

using namespace drone::router;

TEST(RouterConfigGeneratorTest, FullActiveMode) {
    RouterConfig cfg;
    cfg.fc_port = "/dev/ttyAMA4";
    cfg.fc_baud = 921600;
    cfg.siyi_port = "/dev/ttyAMA0";
    cfg.siyi_baud = 115200;
    cfg.siyi_enabled = true;
    cfg.gcs_ip = "192.168.10.50";
    cfg.tcp_port = 5760;

    std::string conf = RouterSupervisor::generate_config_content(cfg, true, true);

    // Verify all endpoints and TCP server port are generated
    EXPECT_NE(conf.find("TcpServerPort = 5760"), std::string::npos);

    EXPECT_NE(conf.find("[UartEndpoint FlightController]"), std::string::npos);
    EXPECT_NE(conf.find("Device = /dev/ttyAMA4"), std::string::npos);
    EXPECT_NE(conf.find("Baud = 921600"), std::string::npos);

    EXPECT_NE(conf.find("[UartEndpoint SIYI_Serial]"), std::string::npos);
    EXPECT_NE(conf.find("Device = /dev/ttyAMA0"), std::string::npos);
    EXPECT_NE(conf.find("Baud = 115200"), std::string::npos);

    EXPECT_NE(conf.find("[UdpEndpoint QGC_UDP_Server]"), std::string::npos);
    EXPECT_NE(conf.find("Port = 14550"), std::string::npos);

    EXPECT_NE(conf.find("[UdpEndpoint ConfigAgent]"), std::string::npos);
    EXPECT_NE(conf.find("Port = 14600"), std::string::npos);

    EXPECT_NE(conf.find("[UdpEndpoint MAVROS_Companion]"), std::string::npos);
    EXPECT_NE(conf.find("Port = 14541"), std::string::npos);

    EXPECT_NE(conf.find("[UdpEndpoint QGC_Direct]"), std::string::npos);
    EXPECT_NE(conf.find("Address = 192.168.10.50"), std::string::npos);
}

TEST(RouterConfigGeneratorTest, GracefulStandbyModeWhenFcAbsent) {
    RouterConfig cfg;
    cfg.fc_port = "/dev/ttyAMA4";
    cfg.fc_baud = 921600;
    cfg.siyi_enabled = false;
    cfg.gcs_ip = "";
    cfg.tcp_port = 5760;

    // fc_valid is false (simulating missing port)
    std::string conf = RouterSupervisor::generate_config_content(cfg, false, false);

    // TCP port and UDP endpoints must still be active in Standby
    EXPECT_NE(conf.find("TcpServerPort = 5760"), std::string::npos);

    // FC UART endpoint must NOT be generated to prevent mavlink-routerd crash
    EXPECT_EQ(conf.find("[UartEndpoint FlightController]"), std::string::npos);
    EXPECT_NE(conf.find("Standby: Port /dev/ttyAMA4 not active"), std::string::npos);

    // Core UDP endpoints must still be active
    EXPECT_NE(conf.find("[UdpEndpoint QGC_UDP_Server]"), std::string::npos);
    EXPECT_NE(conf.find("[UdpEndpoint ConfigAgent]"), std::string::npos);
    EXPECT_NE(conf.find("[UdpEndpoint MAVROS_Companion]"), std::string::npos);
}

TEST(RouterConfigGeneratorTest, SiyiDisabledMode) {
    RouterConfig cfg;
    cfg.fc_port = "/dev/ttyAMA4";
    cfg.siyi_port = "/dev/ttyAMA0";
    cfg.siyi_enabled = false; // explicitly disabled

    std::string conf = RouterSupervisor::generate_config_content(cfg, true, true);

    EXPECT_EQ(conf.find("[UartEndpoint SIYI_Serial]"), std::string::npos);
}
