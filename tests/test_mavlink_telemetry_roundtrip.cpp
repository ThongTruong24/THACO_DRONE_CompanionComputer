// Wire-format guard for the telemetry QGC decodes on its "Config" panel.
//
// The agent builds CC_TELEMETRY_LINKS with a long POSITIONAL pack call in
// src/modules/cc_agent_legacy/src/mavlink_core.cpp (send_telemetry_links()). If the argument
// order there drifts from the dialect field order — or the dialect is
// regenerated with a different layout — QGC shows wrong ports/baud/status when
// you open Config. These tests pack with the exact positional order the agent
// uses and assert every value lands in the field QGC reads by name.
#include <gtest/gtest.h>
#include <cstring>

#include "thaco/mavlink.h"

// Mirrors the positional pack in MavlinkCore::send_telemetry_links().
// Distinct sentinel values per field so any swap/misalignment is detectable.
TEST(CcTelemetryLinksRoundtrip, PositionalPackMapsToNamedFields) {
    const char* fc_port         = "/dev/ttyAMA4";
    const char* siyi_port       = "/dev/ttyAMA0";
    const char* available_ports = "ttyAMA0,ttyAMA4,ttyUSB0";

    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_links_pack(
        /*sys*/ 1, /*comp*/ 191, &msg,
        /*fc_tx_rate*/        716.0f,
        /*fc_rx_rate*/        10.0f,
        /*fc_tx_rate_max*/    900.0f,
        /*fc_tx_rate_multi*/  1.0f,
        /*fc_rx_loss*/        2.5f,
        /*fc_tx_err*/         3u,
        /*fc_bytes_rx*/       49810000u,
        /*fc_bytes_tx*/       17140000u,
        /*fc_baudrate*/       921600u,
        /*siyi_tx_rate*/      0.0f,
        /*siyi_rx_rate*/      21.0f,
        /*siyi_tx_rate_max*/  50.0f,
        /*siyi_tx_rate_multi*/1.0f,
        /*siyi_rx_loss*/      0.0f,
        /*siyi_tx_err*/       7u,
        /*siyi_bytes_rx*/     177700u,
        /*siyi_bytes_tx*/     49810000u,
        /*siyi_baudrate*/     115200u,
        /*fc_status*/         2,
        /*siyi_status*/       2,
        /*transport_type*/    1,
        fc_port, siyi_port, available_ports);

    mavlink_cc_telemetry_links_t out;
    mavlink_msg_cc_telemetry_links_decode(&msg, &out);

    // FC block
    EXPECT_FLOAT_EQ(out.fc_tx_rate, 716.0f);
    EXPECT_FLOAT_EQ(out.fc_rx_rate, 10.0f);
    EXPECT_FLOAT_EQ(out.fc_rx_loss, 2.5f);
    EXPECT_EQ(out.fc_tx_err, 3u);
    EXPECT_EQ(out.fc_bytes_rx, 49810000u);
    EXPECT_EQ(out.fc_bytes_tx, 17140000u);
    EXPECT_EQ(out.fc_baudrate, 921600u);
    EXPECT_EQ(out.fc_status, 2);

    // SIYI block — must not be cross-contaminated with FC values
    EXPECT_FLOAT_EQ(out.siyi_tx_rate, 0.0f);
    EXPECT_FLOAT_EQ(out.siyi_rx_rate, 21.0f);
    EXPECT_EQ(out.siyi_tx_err, 7u);
    EXPECT_EQ(out.siyi_bytes_rx, 177700u);
    EXPECT_EQ(out.siyi_baudrate, 115200u);
    EXPECT_EQ(out.siyi_status, 2);

    EXPECT_EQ(out.transport_type, 1);

    // Port strings land in the right fields, are not swapped, and keep full path.
    EXPECT_STREQ(out.fc_port, "/dev/ttyAMA4");
    EXPECT_STREQ(out.siyi_port, "/dev/ttyAMA0");
    EXPECT_STREQ(out.available_ports, "ttyAMA0,ttyAMA4,ttyUSB0");

    // Guard against FC/SIYI baud or port being swapped (the two differ).
    EXPECT_NE(out.fc_baudrate, out.siyi_baudrate);
    EXPECT_STRNE(out.fc_port, out.siyi_port);
}

// A full 16-byte device path must survive the fixed-width char field intact.
TEST(CcTelemetryLinksRoundtrip, LongPortPathNotCorrupted) {
    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_links_pack(
        1, 191, &msg,
        0, 0, 0, 1.0f, 0, 0, 0, 0, 57600u,
        0, 0, 0, 1.0f, 0, 0, 0, 0, 230400u,
        0, 0, 1,
        "/dev/ttyUSB0", "/dev/ttyUSB1", "");

    mavlink_cc_telemetry_links_t out;
    mavlink_msg_cc_telemetry_links_decode(&msg, &out);

    EXPECT_STREQ(out.fc_port, "/dev/ttyUSB0");
    EXPECT_STREQ(out.siyi_port, "/dev/ttyUSB1");
    EXPECT_EQ(out.fc_baudrate, 57600u);
    EXPECT_EQ(out.siyi_baudrate, 230400u);
}

// CC_SERIAL_LINK (42015): one message per configured link; the user-defined name
// and port travel as data. Pack order follows the XML field order (generated
// header is the source of truth; wire order is size-sorted but pack is XML order).
TEST(CcSerialLinkRoundtrip, NameAndPortSurvive) {
    mavlink_message_t msg;
    mavlink_msg_cc_serial_link_pack(1, 191, &msg, 1200.f, 300.f, 0.5f, 1000u, 200u, 3u, 921600u,
                                    0, 2, CC_LINK_STATUS_CONNECTED, "FC", "/dev/ttyAMA4");
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    mavlink_message_t out;
    mavlink_status_t st{};
    bool got = false;
    for (uint16_t i = 0; i < len; ++i) got = mavlink_parse_char(MAVLINK_COMM_1, buf[i], &out, &st) || got;
    ASSERT_TRUE(got);
    EXPECT_EQ(out.msgid, 42015u);

    mavlink_cc_serial_link_t d;
    mavlink_msg_cc_serial_link_decode(&out, &d);
    EXPECT_STREQ(d.name, "FC");
    EXPECT_STREQ(d.port, "/dev/ttyAMA4");
    EXPECT_EQ(d.link_index, 0);
    EXPECT_EQ(d.link_count, 2);
    EXPECT_EQ(d.status, CC_LINK_STATUS_CONNECTED);
    EXPECT_EQ(d.baudrate, 921600u);
    EXPECT_EQ(d.rx_errors, 3u);
    EXPECT_FLOAT_EQ(d.rx_rate, 1200.f);
}
