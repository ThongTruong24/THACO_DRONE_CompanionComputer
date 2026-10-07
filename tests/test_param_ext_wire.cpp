// PARAM_EXT wire-format roundtrip, compiled against the SAME MAVLink dialect
// that cc-agent uses (src/lib/mavlink). This locks the agent's encode convention:
// scalars as raw little-endian bytes in param_value[128], strings NUL-padded.
//
// NOTE: this deliberately does NOT use the mavros-generated c_library, whose
// aligned-pack path uses strncpy for char[] (mav_array_assign_char) and therefore
// corrupts binary values that begin with a 0x00 byte. The agent's dialect uses a
// plain memcpy, which is correct; see the interop note in the task summary.
#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

#include "thaco/mavlink.h"

namespace {
mavlink_message_t wire(const mavlink_message_t& in) {
    uint8_t buf[300];
    uint16_t len = mavlink_msg_to_send_buffer(buf, &in);
    mavlink_message_t rx{}; mavlink_status_t st{};
    for (uint16_t i = 0; i < len; ++i) mavlink_parse_char(MAVLINK_COMM_0, buf[i], &rx, &st);
    return rx;
}
} // namespace

TEST(ParamExtWire, Real32ValueSurvives) {
    char value[128]; std::memset(value, 0, sizeof(value));
    float in = 0.55f; std::memcpy(value, &in, sizeof(in));

    mavlink_message_t msg;
    mavlink_msg_param_ext_value_pack(1, 191, &msg, "CC_VIS_CONF", value, 9 /*REAL32*/, 14, 7);
    mavlink_message_t rx = wire(msg);
    mavlink_param_ext_value_t out;
    mavlink_msg_param_ext_value_decode(&rx, &out);

    float got; std::memcpy(&got, out.param_value, sizeof(got));
    EXPECT_FLOAT_EQ(got, 0.55f);
    EXPECT_EQ(out.param_type, 9);
    EXPECT_EQ(out.param_count, 14);
    EXPECT_EQ(out.param_index, 7);
    EXPECT_STREQ(out.param_id, "CC_VIS_CONF");
}

// Regression: binary scalars whose little-endian encoding begins with 0x00 must
// survive (921600 -> 00 10 0e 00, 57600 -> 00 e1 00 00).
TEST(ParamExtWire, Uint32ValuesWithLeadingZeroByteSurvive) {
    for (uint32_t in : {921600u, 57600u, 115200u}) {
        char value[128]; std::memset(value, 0, sizeof(value));
        std::memcpy(value, &in, sizeof(in));
        mavlink_message_t msg;
        mavlink_msg_param_ext_set_pack(1, 191, &msg, 1, 191, "CC_FC_BAUD", value, 5 /*UINT32*/);
        mavlink_message_t rx = wire(msg);
        mavlink_param_ext_set_t out;
        mavlink_msg_param_ext_set_decode(&rx, &out);
        uint32_t got; std::memcpy(&got, out.param_value, sizeof(got));
        EXPECT_EQ(got, in);
    }
}

TEST(ParamExtWire, CustomStringValueSurvives) {
    char value[128]; std::memset(value, 0, sizeof(value));
    std::strncpy(value, "/dev/ttyUSB0", sizeof(value) - 1);
    mavlink_message_t msg;
    mavlink_msg_param_ext_set_pack(1, 191, &msg, 1, 191, "CC_FC_PORT", value, 11 /*CUSTOM*/);
    mavlink_message_t rx = wire(msg);
    mavlink_param_ext_set_t out;
    mavlink_msg_param_ext_set_decode(&rx, &out);

    char got[129]; std::memcpy(got, out.param_value, 128); got[128] = '\0';
    EXPECT_STREQ(got, "/dev/ttyUSB0");
    EXPECT_STREQ(out.param_id, "CC_FC_PORT");
}
