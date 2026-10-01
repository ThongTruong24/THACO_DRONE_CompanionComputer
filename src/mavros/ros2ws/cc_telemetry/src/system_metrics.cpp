#include "cc_telemetry/system_metrics.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace cc {

SystemMetrics::SystemMetrics() {
  read_cpu_raw(prev_cpu_samples_);
  update();
}

void SystemMetrics::read_cpu_raw(std::map<std::string, CpuTickSample> & samples) {
  std::ifstream file("/proc/stat");
  if (!file.is_open()) return;

  std::string line;
  while (std::getline(file, line)) {
    if (line.rfind("cpu", 0) == 0) {
      std::istringstream iss(line);
      std::string name;
      uint64_t user{0}, nice{0}, system{0}, idle{0}, iowait{0}, irq{0}, softirq{0}, steal{0};
      if (iss >> name >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) {
        CpuTickSample s;
        s.idle = idle + iowait;
        s.total = user + nice + system + idle + iowait + irq + softirq + steal;
        samples[name] = s;
      }
    }
  }
}

void SystemMetrics::read_temperature() {
  std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
  if (!file.is_open()) {
    file.open("/sys/devices/virtual/thermal/thermal_zone0/temp");
  }
  if (!file.is_open()) {
    temp_c_ = 0.0f;
    return;
  }

  float raw_val = 0.0f;
  if (file >> raw_val) {
    temp_c_ = raw_val / 1000.0f;
  }
}

void SystemMetrics::read_memory() {
  std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    ram_used_mb_ = ram_total_mb_ = 0;
    ram_pct_ = 0.0f;
    return;
  }

  std::string key, unit;
  uint64_t val = 0;
  uint64_t mem_total_kb = 0, mem_avail_kb = 0;

  while (file >> key >> val >> unit) {
    if (key == "MemTotal:") mem_total_kb = val;
    else if (key == "MemAvailable:") mem_avail_kb = val;
  }

  ram_total_mb_ = mem_total_kb / 1024;
  ram_used_mb_ = (mem_total_kb > mem_avail_kb) ? (mem_total_kb - mem_avail_kb) / 1024 : 0;
  ram_pct_ = ram_total_mb_ > 0 ? (static_cast<float>(ram_used_mb_) / static_cast<float>(ram_total_mb_)) * 100.0f : 0.0f;
}

void SystemMetrics::update() {
  std::map<std::string, CpuTickSample> cur_samples;
  read_cpu_raw(cur_samples);

  std::map<std::string, float> cpu_percentages;
  for (const auto & [name, cur] : cur_samples) {
    if (prev_cpu_samples_.find(name) != prev_cpu_samples_.end()) {
      const auto & prev = prev_cpu_samples_[name];
      uint64_t total_diff = cur.total - prev.total;
      uint64_t idle_diff = cur.idle - prev.idle;
      if (total_diff > 0) {
        float usage = (1.0f - static_cast<float>(idle_diff) / static_cast<float>(total_diff)) * 100.0f;
        cpu_percentages[name] = std::max(0.0f, std::min(100.0f, usage));
      }
    }
  }
  prev_cpu_samples_ = cur_samples;

  if (cpu_percentages.find("cpu") != cpu_percentages.end()) {
    total_cpu_ = cpu_percentages["cpu"];
  }

  per_core_cpu_.clear();
  for (int core = 0; core < 8; ++core) {
    std::string core_key = "cpu" + std::to_string(core);
    if (cpu_percentages.find(core_key) != cpu_percentages.end()) {
      per_core_cpu_.push_back(cpu_percentages[core_key]);
    }
  }

  read_temperature();
  read_memory();
}

}  // namespace cc
