// MESSAGE CC_TELEMETRY_LINKS support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief CC_TELEMETRY_LINKS message
 *
 * Companion Computer serial and communication telemetry link status for FC and SIYI Air Unit.
 */
struct CC_TELEMETRY_LINKS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42010;
    static constexpr size_t LENGTH = 155;
    static constexpr size_t MIN_LENGTH = 155;
    static constexpr uint8_t CRC_EXTRA = 101;
    static constexpr auto NAME = "CC_TELEMETRY_LINKS";


    float fc_tx_rate; /*<  FC Transmit rate in Bytes/sec */
    float fc_rx_rate; /*<  FC Receive rate in Bytes/sec */
    float fc_tx_rate_max; /*<  FC Peak transmit rate in Bytes/sec */
    float fc_tx_rate_multi; /*<  FC Transmit rate multiplier (e.g. 1.0) */
    float fc_rx_loss; /*<  FC RX packet loss percentage (0.0 - 100.0) */
    uint32_t fc_tx_err; /*<  FC Transmit error count */
    uint32_t fc_bytes_rx; /*<  Total bytes received from Flight Controller */
    uint32_t fc_bytes_tx; /*<  Total bytes transmitted to Flight Controller */
    uint32_t fc_baudrate; /*<  Flight Controller UART baudrate, e.g. 921600 */
    float siyi_tx_rate; /*<  SIYI Transmit rate in Bytes/sec */
    float siyi_rx_rate; /*<  SIYI Receive rate in Bytes/sec */
    float siyi_tx_rate_max; /*<  SIYI Peak transmit rate in Bytes/sec */
    float siyi_tx_rate_multi; /*<  SIYI Transmit rate multiplier (e.g. 1.0) */
    float siyi_rx_loss; /*<  SIYI RX packet loss percentage (0.0 - 100.0) */
    uint32_t siyi_tx_err; /*<  SIYI Transmit error count */
    uint32_t siyi_bytes_rx; /*<  Total bytes received from SIYI link */
    uint32_t siyi_bytes_tx; /*<  Total bytes transmitted to SIYI link */
    uint32_t siyi_baudrate; /*<  SIYI telemetry link baudrate, e.g. 115200 */
    uint8_t fc_status; /*<  Flight Controller link status */
    uint8_t siyi_status; /*<  SIYI Air Unit link status */
    uint8_t transport_type; /*<  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP) */
    std::array<char, 16> fc_port; /*<  FC active serial device node, e.g. /dev/ttyAMA4 */
    std::array<char, 16> siyi_port; /*<  SIYI active serial device node, e.g. /dev/ttyAMA0 */
    std::array<char, 48> available_ports; /*<  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4 */


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
        ss << "  fc_tx_rate: " << fc_tx_rate << std::endl;
        ss << "  fc_rx_rate: " << fc_rx_rate << std::endl;
        ss << "  fc_tx_rate_max: " << fc_tx_rate_max << std::endl;
        ss << "  fc_tx_rate_multi: " << fc_tx_rate_multi << std::endl;
        ss << "  fc_rx_loss: " << fc_rx_loss << std::endl;
        ss << "  fc_tx_err: " << fc_tx_err << std::endl;
        ss << "  fc_bytes_rx: " << fc_bytes_rx << std::endl;
        ss << "  fc_bytes_tx: " << fc_bytes_tx << std::endl;
        ss << "  fc_baudrate: " << fc_baudrate << std::endl;
        ss << "  siyi_tx_rate: " << siyi_tx_rate << std::endl;
        ss << "  siyi_rx_rate: " << siyi_rx_rate << std::endl;
        ss << "  siyi_tx_rate_max: " << siyi_tx_rate_max << std::endl;
        ss << "  siyi_tx_rate_multi: " << siyi_tx_rate_multi << std::endl;
        ss << "  siyi_rx_loss: " << siyi_rx_loss << std::endl;
        ss << "  siyi_tx_err: " << siyi_tx_err << std::endl;
        ss << "  siyi_bytes_rx: " << siyi_bytes_rx << std::endl;
        ss << "  siyi_bytes_tx: " << siyi_bytes_tx << std::endl;
        ss << "  siyi_baudrate: " << siyi_baudrate << std::endl;
        ss << "  fc_status: " << +fc_status << std::endl;
        ss << "  siyi_status: " << +siyi_status << std::endl;
        ss << "  transport_type: " << +transport_type << std::endl;
        ss << "  fc_port: \"" << to_string(fc_port) << "\"" << std::endl;
        ss << "  siyi_port: \"" << to_string(siyi_port) << "\"" << std::endl;
        ss << "  available_ports: \"" << to_string(available_ports) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << fc_tx_rate;                    // offset: 0
        map << fc_rx_rate;                    // offset: 4
        map << fc_tx_rate_max;                // offset: 8
        map << fc_tx_rate_multi;              // offset: 12
        map << fc_rx_loss;                    // offset: 16
        map << fc_tx_err;                     // offset: 20
        map << fc_bytes_rx;                   // offset: 24
        map << fc_bytes_tx;                   // offset: 28
        map << fc_baudrate;                   // offset: 32
        map << siyi_tx_rate;                  // offset: 36
        map << siyi_rx_rate;                  // offset: 40
        map << siyi_tx_rate_max;              // offset: 44
        map << siyi_tx_rate_multi;            // offset: 48
        map << siyi_rx_loss;                  // offset: 52
        map << siyi_tx_err;                   // offset: 56
        map << siyi_bytes_rx;                 // offset: 60
        map << siyi_bytes_tx;                 // offset: 64
        map << siyi_baudrate;                 // offset: 68
        map << fc_status;                     // offset: 72
        map << siyi_status;                   // offset: 73
        map << transport_type;                // offset: 74
        map << fc_port;                       // offset: 75
        map << siyi_port;                     // offset: 91
        map << available_ports;               // offset: 107
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> fc_tx_rate;                    // offset: 0
        map >> fc_rx_rate;                    // offset: 4
        map >> fc_tx_rate_max;                // offset: 8
        map >> fc_tx_rate_multi;              // offset: 12
        map >> fc_rx_loss;                    // offset: 16
        map >> fc_tx_err;                     // offset: 20
        map >> fc_bytes_rx;                   // offset: 24
        map >> fc_bytes_tx;                   // offset: 28
        map >> fc_baudrate;                   // offset: 32
        map >> siyi_tx_rate;                  // offset: 36
        map >> siyi_rx_rate;                  // offset: 40
        map >> siyi_tx_rate_max;              // offset: 44
        map >> siyi_tx_rate_multi;            // offset: 48
        map >> siyi_rx_loss;                  // offset: 52
        map >> siyi_tx_err;                   // offset: 56
        map >> siyi_bytes_rx;                 // offset: 60
        map >> siyi_bytes_tx;                 // offset: 64
        map >> siyi_baudrate;                 // offset: 68
        map >> fc_status;                     // offset: 72
        map >> siyi_status;                   // offset: 73
        map >> transport_type;                // offset: 74
        map >> fc_port;                       // offset: 75
        map >> siyi_port;                     // offset: 91
        map >> available_ports;               // offset: 107
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
