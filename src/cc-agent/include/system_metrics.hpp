#pragma once

#include <cstdint>

namespace cc {

struct SystemStats {
    uint32_t uptime_seconds = 0;
    uint8_t  cpu_usage_pct = 0;
    uint8_t  ram_usage_pct = 0;
    uint8_t  disk_usage_pct = 0;
    int8_t   cpu_temp_celsius = 0;
};

class SystemMetricsCollector {
public:
    SystemMetricsCollector();
    SystemStats read_stats();

private:
    uint64_t prev_idle_ = 0;
    uint64_t prev_total_ = 0;
};

} // namespace cc
