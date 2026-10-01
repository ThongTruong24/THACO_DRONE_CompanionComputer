#include "system_metrics.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <sys/statvfs.h>

namespace cc {

SystemMetricsCollector::SystemMetricsCollector() {
    // Prime the first CPU reading
    read_stats();
}

SystemStats SystemMetricsCollector::read_stats() {
    SystemStats stats;

    // 1. Uptime
    {
        std::ifstream f("/proc/uptime");
        if (f.is_open()) {
            double up = 0;
            f >> up;
            stats.uptime_seconds = static_cast<uint32_t>(up);
        }
    }

    // 2. CPU Usage
    {
        std::ifstream f("/proc/stat");
        if (f.is_open()) {
            std::string line;
            std::getline(f, line);
            std::istringstream ss(line);
            std::string cpu_lbl;
            uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
            if (ss >> cpu_lbl >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) {
                uint64_t idle_all = idle + iowait;
                uint64_t non_idle = user + nice + system + irq + softirq + steal;
                uint64_t total = idle_all + non_idle;

                uint64_t total_diff = total - prev_total_;
                uint64_t idle_diff = idle_all - prev_idle_;

                if (total_diff > 0) {
                    double usage = 100.0 * static_cast<double>(total_diff - idle_diff) / static_cast<double>(total_diff);
                    stats.cpu_usage_pct = static_cast<uint8_t>(usage > 100.0 ? 100.0 : (usage < 0.0 ? 0.0 : usage));
                }
                prev_total_ = total;
                prev_idle_ = idle_all;
            }
        }
    }

    // 3. RAM Usage
    {
        std::ifstream f("/proc/meminfo");
        if (f.is_open()) {
            std::string line;
            uint64_t mem_total = 0, mem_avail = 0;
            while (std::getline(f, line)) {
                if (line.rfind("MemTotal:", 0) == 0) {
                    std::sscanf(line.c_str(), "MemTotal: %lu kB", &mem_total);
                } else if (line.rfind("MemAvailable:", 0) == 0) {
                    std::sscanf(line.c_str(), "MemAvailable: %lu kB", &mem_avail);
                }
            }
            if (mem_total > 0 && mem_avail <= mem_total) {
                stats.ram_usage_pct = static_cast<uint8_t>(100 * (mem_total - mem_avail) / mem_total);
            }
        }
    }

    // 4. Disk Usage
    {
        struct statvfs st;
        if (statvfs("/", &st) == 0) {
            uint64_t total_blocks = st.f_blocks;
            uint64_t free_blocks = st.f_bavail;
            if (total_blocks > 0) {
                stats.disk_usage_pct = static_cast<uint8_t>(100 * (total_blocks - free_blocks) / total_blocks);
            }
        }
    }

    // 5. CPU Temperature
    {
        std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
        if (f.is_open()) {
            int32_t milli_deg = 0;
            f >> milli_deg;
            stats.cpu_temp_celsius = static_cast<int8_t>(milli_deg / 1000);
        }
    }

    return stats;
}

} // namespace cc
