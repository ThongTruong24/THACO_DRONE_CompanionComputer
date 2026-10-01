#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace cc {

struct CpuTickSample {
  uint64_t idle{0};
  uint64_t total{0};
};

class SystemMetrics {
 public:
  SystemMetrics();

  void update();

  float total_cpu() const { return total_cpu_; }
  const std::vector<float> & per_core_cpu() const { return per_core_cpu_; }
  float ram_percentage() const { return ram_pct_; }
  uint64_t ram_used_mb() const { return ram_used_mb_; }
  uint64_t ram_total_mb() const { return ram_total_mb_; }
  float temperature_c() const { return temp_c_; }

 private:
  void read_cpu_raw(std::map<std::string, CpuTickSample> & samples);
  void read_memory();
  void read_temperature();

  std::map<std::string, CpuTickSample> prev_cpu_samples_;
  float total_cpu_{0.0f};
  std::vector<float> per_core_cpu_;
  uint64_t ram_used_mb_{0};
  uint64_t ram_total_mb_{0};
  float ram_pct_{0.0f};
  float temp_c_{0.0f};
};

}  // namespace cc
