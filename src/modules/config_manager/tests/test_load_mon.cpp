#include <gtest/gtest.h>
#include "load_mon.h"

using cc::LoadMon;

TEST(LoadMonTest, ReadMetricsReturnsValidRanges)
{
	LoadMon mon;
	auto metrics = mon.read_metrics();

	// Linux host /proc/uptime should be > 0
	EXPECT_GT(metrics.uptime_s, 0u);

	// Percentages must be between 0 and 100
	EXPECT_LE(metrics.cpu_usage, 100);
	EXPECT_LE(metrics.ram_usage, 100);
	EXPECT_LE(metrics.disk_usage, 100);

	// Temp is typically in reasonable range (e.g. -40C to 125C, or 0 if not present)
	EXPECT_GE(metrics.cpu_temp, -40);
	EXPECT_LE(metrics.cpu_temp, 125);
}
