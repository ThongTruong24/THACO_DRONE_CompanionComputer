#include <gtest/gtest.h>
#include "mavlink_router/router_stats_parser.hpp"

using namespace cc;

TEST(RouterStatsParserTest, ParseEndpointStats)
{
	RouterStatsParser parser;
	std::vector<std::string> lines = {
		"UART Endpoint [3] Radio {",
		"\tReceived messages {",
		"\t\tCRC error: 2 1% 10KB",
		"\t\tSequence lost: 5 2%",
		"\t\tHandled: 1200 45KB",
		"\t\tTotal: 1205",
		"\t}",
		"\tTransmitted messages {",
		"\t\tTotal: 800 30KB",
		"\t}",
		"}"
	};

	for (const auto &line : lines) {
		parser.feed_line(line);
	}

	auto stats = parser.get_endpoint_stats();
	ASSERT_TRUE(stats.find("Radio") != stats.end());

	const auto &st = stats.at("Radio");
	EXPECT_EQ(st.rx_bytes, 45000u);
	EXPECT_EQ(st.tx_bytes, 30000u);
	EXPECT_FLOAT_EQ(st.rx_loss_pct, 2.0f);
	EXPECT_TRUE(st.online);
}

TEST(RouterStatsParserTest, ResetClearsAllEndpoints)
{
	RouterStatsParser parser;
	parser.feed_line("UART Endpoint [1] Telemetry {");
	parser.feed_line("}");
	EXPECT_EQ(parser.get_endpoint_stats().size(), 1u);

	parser.reset();
	EXPECT_EQ(parser.get_endpoint_stats().size(), 0u);
}
