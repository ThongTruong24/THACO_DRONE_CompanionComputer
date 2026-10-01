// MESSAGE CC_CONFIG_SET support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief CC_CONFIG_SET message
 *
 * Stage a configuration key-value update inside active transaction.
 */
struct CC_CONFIG_SET : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42101;
    static constexpr size_t LENGTH = 200;
    static constexpr size_t MIN_LENGTH = 200;
    static constexpr uint8_t CRC_EXTRA = 182;
    static constexpr auto NAME = "CC_CONFIG_SET";


    uint32_t request_id; /*<  Client request ID */
    uint32_t transaction_id; /*<  Active transaction ID */
    std::array<char, 64> key; /*<  Namespaced configuration key path */
    std::array<char, 128> value; /*<  New value encoded as string */


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
        ss << "  key: \"" << to_string(key) << "\"" << std::endl;
        ss << "  value: \"" << to_string(value) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << transaction_id;                // offset: 4
        map << key;                           // offset: 8
        map << value;                         // offset: 72
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> transaction_id;                // offset: 4
        map >> key;                           // offset: 8
        map >> value;                         // offset: 72
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
