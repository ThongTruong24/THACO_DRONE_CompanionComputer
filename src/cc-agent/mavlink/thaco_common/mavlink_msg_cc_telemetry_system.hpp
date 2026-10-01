// MESSAGE CC_TELEMETRY_SYSTEM support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief CC_TELEMETRY_SYSTEM message
 *
 * Companion Computer system health, SoC temperature, CPU/RAM/Disk resources, and uptime.
 */
struct CC_TELEMETRY_SYSTEM : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42014;
    static constexpr size_t LENGTH = 9;
    static constexpr size_t MIN_LENGTH = 9;
    static constexpr uint8_t CRC_EXTRA = 159;
    static constexpr auto NAME = "CC_TELEMETRY_SYSTEM";


    uint32_t system_uptime_s; /*<  System uptime since boot in seconds */
    uint8_t cpu_usage; /*<  CPU utilization percentage (0 - 100) */
    uint8_t ram_usage; /*<  RAM memory utilization percentage (0 - 100) */
    uint8_t disk_usage; /*<  Storage eMMC/SD card utilization percentage (0 - 100) */
    int8_t cpu_temp; /*<  CPU SoC temperature in degrees Celsius */
    uint8_t system_status; /*<  System health alerts bitmask */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  system_uptime_s: " << system_uptime_s << std::endl;
        ss << "  cpu_usage: " << +cpu_usage << std::endl;
        ss << "  ram_usage: " << +ram_usage << std::endl;
        ss << "  disk_usage: " << +disk_usage << std::endl;
        ss << "  cpu_temp: " << +cpu_temp << std::endl;
        ss << "  system_status: " << +system_status << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << system_uptime_s;               // offset: 0
        map << cpu_usage;                     // offset: 4
        map << ram_usage;                     // offset: 5
        map << disk_usage;                    // offset: 6
        map << cpu_temp;                      // offset: 7
        map << system_status;                 // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> system_uptime_s;               // offset: 0
        map >> cpu_usage;                     // offset: 4
        map >> ram_usage;                     // offset: 5
        map >> disk_usage;                    // offset: 6
        map >> cpu_temp;                      // offset: 7
        map >> system_status;                 // offset: 8
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
