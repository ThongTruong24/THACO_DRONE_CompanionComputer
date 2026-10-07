#include <gtest/gtest.h>
#include "mavlink_stream.h"

namespace
{
class MockMavlinkStream : public MavlinkStream
{
public:
	explicit MockMavlinkStream(uint32_t interval_us = 1000)
		: MavlinkStream(nullptr)
	{
		set_interval(interval_us);
	}

	const char *get_name() const override { return "MOCK"; }
	uint16_t get_id() override { return 999; }
	unsigned get_size() override { return 10; }

	bool send() override
	{
		++send_call_count;
		return send_return_value;
	}

	bool send_return_value{true};
	int send_call_count{0};
};
} // namespace

TEST(MavlinkStreamTest, SendsAtConfiguredInterval)
{
	MockMavlinkStream stream(1000); // 1000 us

	// t = 0: first update, should send
	stream.update(0);
	EXPECT_EQ(stream.send_call_count, 1);
	EXPECT_EQ(stream.get_last_sent(), 0u);

	// t = 500: not enough time elapsed, should not send
	stream.update(500);
	EXPECT_EQ(stream.send_call_count, 1);

	// t = 1000: interval elapsed, should send
	stream.update(1000);
	EXPECT_EQ(stream.send_call_count, 2);
	EXPECT_EQ(stream.get_last_sent(), 1000u);
}

TEST(MavlinkStreamTest, SendFailureDoesNotCountAsSent)
{
	MockMavlinkStream stream(1000);
	stream.set_last_sent(0);

	// Make send() return false
	stream.send_return_value = false;

	// At t = 1000, send() fails
	stream.update(1000);
	EXPECT_EQ(stream.send_call_count, 1);
	// _last_sent must NOT be updated because send() returned false
	EXPECT_EQ(stream.get_last_sent(), 0u);

	// At t = 1050, it should try again immediately because it wasn't sent
	stream.update(1050);
	EXPECT_EQ(stream.send_call_count, 2);
	EXPECT_EQ(stream.get_last_sent(), 0u);

	// Now send() succeeds
	stream.send_return_value = true;
	stream.update(1100);
	EXPECT_EQ(stream.send_call_count, 3);
	EXPECT_EQ(stream.get_last_sent(), 1100u);

	// At t = 1500, interval hasn't elapsed since 1100
	stream.update(1500);
	EXPECT_EQ(stream.send_call_count, 3);
}
