// MESSAGE CC_SERIAL_LINK support class

#pragma once

namespace mavlink {
namespace thaco {
namespace msg {

/**
 * @brief CC_SERIAL_LINK message
 *
 * One companion-computer serial link, sent at 1 Hz for each configured link. The link identity is the user-defined name (data), never a hardcoded role.
 */
struct CC_SERIAL_LINK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42010;
    static constexpr size_t LENGTH = 63;
    static constexpr size_t MIN_LENGTH = 63;
    static constexpr uint8_t CRC_EXTRA = 19;
    static constexpr auto NAME = "CC_SERIAL_LINK";


    float rx_rate; /*< [B/s] MAVLink bytes/s received on this link (router endpoint stats) */
    float tx_rate; /*< [B/s] MAVLink bytes/s transmitted on this link */
    float rx_loss; /*< [%] Sequence loss percentage */
    uint32_t rx_bytes; /*< [bytes] Total bytes received */
    uint32_t tx_bytes; /*< [bytes] Total bytes transmitted */
    uint32_t rx_errors; /*<  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise) */
    uint32_t baudrate; /*< [bps] Configured baud rate */
    uint8_t link_index; /*<  Index of this link in the active link list (0..link_count-1) */
    uint8_t link_count; /*<  Number of configured links */
    uint8_t status; /*<  Link status */
    std::array<char, 16> name; /*<  User-defined link name, e.g. FC */
    std::array<char, 16> port; /*<  Serial device node, e.g. /dev/ttyAMA4 */


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
        ss << "  rx_rate: " << rx_rate << std::endl;
        ss << "  tx_rate: " << tx_rate << std::endl;
        ss << "  rx_loss: " << rx_loss << std::endl;
        ss << "  rx_bytes: " << rx_bytes << std::endl;
        ss << "  tx_bytes: " << tx_bytes << std::endl;
        ss << "  rx_errors: " << rx_errors << std::endl;
        ss << "  baudrate: " << baudrate << std::endl;
        ss << "  link_index: " << +link_index << std::endl;
        ss << "  link_count: " << +link_count << std::endl;
        ss << "  status: " << +status << std::endl;
        ss << "  name: \"" << to_string(name) << "\"" << std::endl;
        ss << "  port: \"" << to_string(port) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << rx_rate;                       // offset: 0
        map << tx_rate;                       // offset: 4
        map << rx_loss;                       // offset: 8
        map << rx_bytes;                      // offset: 12
        map << tx_bytes;                      // offset: 16
        map << rx_errors;                     // offset: 20
        map << baudrate;                      // offset: 24
        map << link_index;                    // offset: 28
        map << link_count;                    // offset: 29
        map << status;                        // offset: 30
        map << name;                          // offset: 31
        map << port;                          // offset: 47
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> rx_rate;                       // offset: 0
        map >> tx_rate;                       // offset: 4
        map >> rx_loss;                       // offset: 8
        map >> rx_bytes;                      // offset: 12
        map >> tx_bytes;                      // offset: 16
        map >> rx_errors;                     // offset: 20
        map >> baudrate;                      // offset: 24
        map >> link_index;                    // offset: 28
        map >> link_count;                    // offset: 29
        map >> status;                        // offset: 30
        map >> name;                          // offset: 31
        map >> port;                          // offset: 47
    }
};

} // namespace msg
} // namespace thaco
} // namespace mavlink
