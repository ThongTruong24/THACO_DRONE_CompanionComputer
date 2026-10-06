// MESSAGE CC_CONFIG_STATUS support class

#pragma once

namespace mavlink {
namespace thaco {
namespace msg {

/**
 * @brief CC_CONFIG_STATUS message
 *
 * Detailed status and progress report of active transaction.
 */
struct CC_CONFIG_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42108;
    static constexpr size_t LENGTH = 58;
    static constexpr size_t MIN_LENGTH = 58;
    static constexpr uint8_t CRC_EXTRA = 156;
    static constexpr auto NAME = "CC_CONFIG_STATUS";


    uint32_t transaction_id; /*<  Active transaction ID */
    uint8_t state; /*<  Current lifecycle state */
    uint8_t progress; /*<  Progress percentage (0 - 100) */
    uint16_t error_code; /*<  Subsystem error code if state is error */
    std::array<char, 50> message; /*<  Status description */


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
        ss << "  transaction_id: " << transaction_id << std::endl;
        ss << "  state: " << +state << std::endl;
        ss << "  progress: " << +progress << std::endl;
        ss << "  error_code: " << error_code << std::endl;
        ss << "  message: \"" << to_string(message) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << transaction_id;                // offset: 0
        map << error_code;                    // offset: 4
        map << state;                         // offset: 6
        map << progress;                      // offset: 7
        map << message;                       // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> transaction_id;                // offset: 0
        map >> error_code;                    // offset: 4
        map >> state;                         // offset: 6
        map >> progress;                      // offset: 7
        map >> message;                       // offset: 8
    }
};

} // namespace msg
} // namespace thaco
} // namespace mavlink
