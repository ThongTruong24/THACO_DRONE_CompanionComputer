// MESSAGE CC_CONFIG_ACK support class

#pragma once

namespace mavlink {
namespace thaco {
namespace msg {

/**
 * @brief CC_CONFIG_ACK message
 *
 * Structured result response from config subsystem to QGC.
 */
struct CC_CONFIG_ACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42106;
    static constexpr size_t LENGTH = 62;
    static constexpr size_t MIN_LENGTH = 62;
    static constexpr uint8_t CRC_EXTRA = 94;
    static constexpr auto NAME = "CC_CONFIG_ACK";


    uint32_t request_id; /*<  Matching client request ID */
    uint32_t transaction_id; /*<  Active transaction ID */
    uint8_t result; /*<  Transaction result status */
    uint8_t apply_type; /*<  Required system apply action */
    uint16_t error_code; /*<  Subsystem error code (0=No error) */
    std::array<char, 50> message; /*<  Human-readable descriptive feedback */


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
        ss << "  result: " << +result << std::endl;
        ss << "  apply_type: " << +apply_type << std::endl;
        ss << "  error_code: " << error_code << std::endl;
        ss << "  message: \"" << to_string(message) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << transaction_id;                // offset: 4
        map << error_code;                    // offset: 8
        map << result;                        // offset: 10
        map << apply_type;                    // offset: 11
        map << message;                       // offset: 12
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> transaction_id;                // offset: 4
        map >> error_code;                    // offset: 8
        map >> result;                        // offset: 10
        map >> apply_type;                    // offset: 11
        map >> message;                       // offset: 12
    }
};

} // namespace msg
} // namespace thaco
} // namespace mavlink
