#include <gtest/gtest.h>
#include "bridge.hpp"

using namespace vision;
TEST(Codec, GeneratedDialectRoundtrip) {
    Codec codec(1, 192);
    Decoder decoder;
    auto frames = decoder.feed(codec.config_get(0x5a170001, "telemetry.fc.baudrate"));
    ASSERT_EQ(frames.size(), 1u);
    const auto& m = frames[0];
    EXPECT_EQ(m.msgid, 42102u);
    EXPECT_EQ(m.sysid, 1);
    EXPECT_EQ(m.compid, 192);
    mavlink_cc_config_get_t get{};
    mavlink_msg_cc_config_get_decode(&m, &get);
    EXPECT_EQ(get.request_id, 0x5a170001u);
    EXPECT_EQ(std::string(get.key), "telemetry.fc.baudrate");
}
TEST(Codec, RejectsMalformedAndTruncatedFrames) {
    Codec codec(1, 192);
    Decoder decoder;
    auto packet = codec.heartbeat();
    auto corrupted = packet;
    corrupted.back() ^= 0xff;
    EXPECT_TRUE(decoder.feed(corrupted).empty());
    decoder.reset();
    EXPECT_TRUE(decoder.feed({packet.begin(), packet.begin() + 5}).empty());
    decoder.reset();
    EXPECT_EQ(decoder.feed(packet).size(), 1u);
}
TEST(Codec, HandlesSplitAndCoalescedStream) {
    Codec codec(1, 192);
    auto a = codec.heartbeat(), b = codec.config_get(123, "telemetry.fc.baudrate");
    Decoder decoder;
    for (size_t i = 0; i + 1 < a.size(); ++i)
        EXPECT_TRUE(decoder.feed({a[i]}).empty());
    EXPECT_EQ(decoder.feed({a.back()}).size(), 1u);
    a.insert(a.end(), b.begin(), b.end());
    EXPECT_EQ(decoder.feed(a).size(), 2u);
}
TEST(Codec, ValidatesConfigurationAndTargets) {
    EXPECT_THROW(Config("127.0.0.1", 0, 1, 192), std::invalid_argument);
    EXPECT_THROW(Config("bad-host", 5760, 1, 192), std::invalid_argument);
    EXPECT_THROW(Config("127.0.0.1", 5760, 0, 192), std::invalid_argument);
    EXPECT_THROW(Config("127.0.0.1", 5760, 1, 256), std::invalid_argument);
    EXPECT_THROW(Config("127.0.0.1", 5760, 1, 191), std::invalid_argument);
    Config c("127.0.0.1", 5760, 1, 192);
    EXPECT_TRUE(c.targets(1, 192));
    EXPECT_FALSE(c.targets(2, 192));
    EXPECT_FALSE(c.targets(1, 191));
    EXPECT_FALSE(c.targets(1, 0));
}
TEST(Queue, BoundedAndLatestOnly) {
    PacketQueue queue(2);
    EXPECT_TRUE(queue.push({1}, true));
    EXPECT_TRUE(queue.push({2}, true));
    EXPECT_EQ(queue.size(), 1u);
    EXPECT_TRUE(queue.push({3}));
    EXPECT_FALSE(queue.push({4}));
    EXPECT_EQ(queue.pop().value(), Bytes({2}));
    EXPECT_EQ(queue.pop().value(), Bytes({3}));
    EXPECT_FALSE(queue.pop().has_value());
    queue.push({5}); queue.clear(); EXPECT_EQ(queue.size(), 0u);
}
TEST(Backoff, BoundedAndResettable) {
    Backoff backoff(10, 40);
    EXPECT_EQ(backoff.next(), 10);
    EXPECT_EQ(backoff.next(), 20);
    EXPECT_EQ(backoff.next(), 40);
    EXPECT_EQ(backoff.next(), 40);
    backoff.reset(); EXPECT_EQ(backoff.next(), 10);
}
