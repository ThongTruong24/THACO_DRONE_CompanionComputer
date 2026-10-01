#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <chrono>

// Include THACO dialect MAVLink root header
#include "thaco_common/mavlink.h"

class MavlinkLoopbackTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create server UDP socket on 127.0.0.1:14777
        server_fd = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_GE(server_fd, 0);

        struct sockaddr_in srv_addr{};
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        srv_addr.sin_port = htons(14777);

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        ASSERT_EQ(bind(server_fd, reinterpret_cast<struct sockaddr*>(&srv_addr), sizeof(srv_addr)), 0);

        // Create client UDP socket
        client_fd = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_GE(client_fd, 0);
    }

    void TearDown() override {
        if (server_fd >= 0) close(server_fd);
        if (client_fd >= 0) close(client_fd);
    }

    int server_fd{-1};
    int client_fd{-1};
};

TEST_F(MavlinkLoopbackTest, HeartbeatEncodeDecodeOverUdp) {
    mavlink_message_t send_msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    // Pack Heartbeat
    uint8_t sys_id = 1;
    uint8_t comp_id = 191; // Companion Computer
    mavlink_msg_heartbeat_pack(
        sys_id, comp_id, &send_msg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_ARDUPILOTMEGA,
        MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        3, // custom_mode
        MAV_STATE_ACTIVE
    );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &send_msg);
    ASSERT_GT(len, 0u);

    // Send UDP
    struct sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr("127.0.0.1");
    dst.sin_port = htons(14777);

    ssize_t sent = sendto(client_fd, buf, len, 0, reinterpret_cast<struct sockaddr*>(&dst), sizeof(dst));
    ASSERT_EQ(sent, len);

    // Receive UDP
    uint8_t recv_buf[MAVLINK_MAX_PACKET_LEN];
    ssize_t recvd = recvfrom(server_fd, recv_buf, sizeof(recv_buf), 0, nullptr, nullptr);
    ASSERT_EQ(recvd, len);

    // Parse packet
    mavlink_message_t recv_msg;
    mavlink_status_t status;
    bool parsed = false;

    for (ssize_t i = 0; i < recvd; ++i) {
        if (mavlink_parse_char(MAVLINK_COMM_0, recv_buf[i], &recv_msg, &status)) {
            parsed = true;
            break;
        }
    }

    ASSERT_TRUE(parsed);
    EXPECT_EQ(recv_msg.msgid, MAVLINK_MSG_ID_HEARTBEAT);
    EXPECT_EQ(recv_msg.sysid, sys_id);
    EXPECT_EQ(recv_msg.compid, comp_id);

    mavlink_heartbeat_t hb;
    mavlink_msg_heartbeat_decode(&recv_msg, &hb);
    EXPECT_EQ(hb.type, MAV_TYPE_QUADROTOR);
    EXPECT_EQ(hb.autopilot, MAV_AUTOPILOT_ARDUPILOTMEGA);
    EXPECT_EQ(hb.system_status, MAV_STATE_ACTIVE);
}

TEST_F(MavlinkLoopbackTest, CustomTelemetrySystemEncodeDecodeOverUdp) {
    mavlink_message_t send_msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    // Pack CC_TELEMETRY_SYSTEM (ID 42014)
    mavlink_cc_telemetry_system_t sys{};
    sys.cpu_usage = 35;
    sys.ram_usage = 52;
    sys.disk_usage = 28;
    sys.cpu_temp = 47;
    sys.system_uptime_s = 3600;

    mavlink_msg_cc_telemetry_system_encode(1, 191, &send_msg, &sys);
    uint16_t len = mavlink_msg_to_send_buffer(buf, &send_msg);
    ASSERT_GT(len, 0u);

    // Send UDP
    struct sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr("127.0.0.1");
    dst.sin_port = htons(14777);

    sendto(client_fd, buf, len, 0, reinterpret_cast<struct sockaddr*>(&dst), sizeof(dst));

    // Receive UDP
    uint8_t recv_buf[MAVLINK_MAX_PACKET_LEN];
    ssize_t recvd = recvfrom(server_fd, recv_buf, sizeof(recv_buf), 0, nullptr, nullptr);
    ASSERT_EQ(recvd, len);

    // Parse packet
    mavlink_message_t recv_msg;
    mavlink_status_t status;
    bool parsed = false;

    for (ssize_t i = 0; i < recvd; ++i) {
        if (mavlink_parse_char(MAVLINK_COMM_1, recv_buf[i], &recv_msg, &status)) {
            parsed = true;
            break;
        }
    }

    ASSERT_TRUE(parsed);
    EXPECT_EQ(recv_msg.msgid, 42014u);

    mavlink_cc_telemetry_system_t sys_out;
    mavlink_msg_cc_telemetry_system_decode(&recv_msg, &sys_out);
    EXPECT_EQ(sys_out.cpu_usage, 35);
    EXPECT_EQ(sys_out.ram_usage, 52);
    EXPECT_EQ(sys_out.disk_usage, 28);
    EXPECT_EQ(sys_out.cpu_temp, 47);
    EXPECT_EQ(sys_out.system_uptime_s, 3600u);
}
