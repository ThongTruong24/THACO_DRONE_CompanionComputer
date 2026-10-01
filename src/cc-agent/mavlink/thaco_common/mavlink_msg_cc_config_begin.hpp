// MESSAGE CC_CONFIG_BEGIN support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief CC_CONFIG_BEGIN message
 *
 * Initiate a new isolated staging configuration transaction.
 */
struct CC_CONFIG_BEGIN : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42100;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 14;
    static constexpr auto NAME = "CC_CONFIG_BEGIN";


    uint32_t request_id; /*<  Client request ID */
    uint32_t timeout_s; /*<  Safety confirmation timeout in seconds (default 30) */


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
        ss << "  request_id: " << request_id << std::endl;
        ss << "  timeout_s: " << timeout_s << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << timeout_s;                     // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> timeout_s;                     // offset: 4
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
