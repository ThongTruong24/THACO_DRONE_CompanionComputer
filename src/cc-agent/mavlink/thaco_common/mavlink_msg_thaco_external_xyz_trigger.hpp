// MESSAGE THACO_EXTERNAL_XYZ_TRIGGER support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief THACO_EXTERNAL_XYZ_TRIGGER message
 *
 * THACO External XYZ trigger - sent immediately and retried once per second while waiting for completion
 */
struct THACO_EXTERNAL_XYZ_TRIGGER : mavlink::Message {
    static constexpr msgid_t MSG_ID = 32000;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 239;
    static constexpr auto NAME = "THACO_EXTERNAL_XYZ_TRIGGER";


    uint32_t trigger_id; /*<  Stable per-handoff trigger ID in range 1..16777216 */
    uint32_t time_boot_ms; /*<  Timestamp in milliseconds since system boot */


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
        ss << "  trigger_id: " << trigger_id << std::endl;
        ss << "  time_boot_ms: " << time_boot_ms << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << trigger_id;                    // offset: 0
        map << time_boot_ms;                  // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> trigger_id;                    // offset: 0
        map >> time_boot_ms;                  // offset: 4
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
