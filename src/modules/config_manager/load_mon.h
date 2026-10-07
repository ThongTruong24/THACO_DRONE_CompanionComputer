#pragma once

#include <cstdint>

namespace cc
{

struct SystemMetrics {
	uint32_t uptime_s{0};
	uint8_t cpu_usage{0};
	uint8_t ram_usage{0};
	uint8_t disk_usage{0};
	int8_t cpu_temp{0};
};

class LoadMon
{
public:
	LoadMon();

	SystemMetrics read_metrics();

private:
	uint64_t _prev_total{0};
	uint64_t _prev_idle{0};
};

} // namespace cc
