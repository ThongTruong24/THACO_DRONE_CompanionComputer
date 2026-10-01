// MESSAGE CC_CONFIG_ROLLBACK support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief CC_CONFIG_ROLLBACK message
 *
 * Explicitly abort active transaction or revert to previous backup.
 */
struct CC_CONFIG_ROLLBACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42105;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 155;
    static constexpr auto NAME = "CC_CONFIG_ROLLBACK";


    uint32_t request_id; /*<  Client request ID */
    uint32_t transaction_id; /*<  Transaction ID */


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
        ss << "  transaction_id: " << transaction_id << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << transaction_id;                // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> transaction_id;                // offset: 4
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
