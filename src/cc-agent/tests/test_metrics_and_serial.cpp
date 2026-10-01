#include <gtest/gtest.h>
#include "system_metrics.hpp"

using namespace cc;

TEST(SystemMetricsCollectorTest, ReadStatsReturnsReasonableValues) {
    SystemMetricsCollector collector;
    SystemStats stats = collector.read_stats();

    // CPU, RAM, Disk percentages should be between 0 and 100
    EXPECT_LE(stats.cpu_usage_pct, 100);
    EXPECT_LE(stats.ram_usage_pct, 100);
    EXPECT_LE(stats.disk_usage_pct, 100);

    // Temp is signed int8 (may be 0 if /sys/class/thermal not present)
    EXPECT_GE(stats.cpu_temp_celsius, -40);
    EXPECT_LE(stats.cpu_temp_celsius, 125);
}
