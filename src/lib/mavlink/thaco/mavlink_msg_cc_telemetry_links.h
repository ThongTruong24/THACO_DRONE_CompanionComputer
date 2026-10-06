#pragma once
// MESSAGE CC_TELEMETRY_LINKS PACKING

#define MAVLINK_MSG_ID_CC_TELEMETRY_LINKS 42010


typedef struct __mavlink_cc_telemetry_links_t {
 float fc_tx_rate; /*<  FC Transmit rate in Bytes/sec*/
 float fc_rx_rate; /*<  FC Receive rate in Bytes/sec*/
 float fc_tx_rate_max; /*<  FC Peak transmit rate in Bytes/sec*/
 float fc_tx_rate_multi; /*<  FC Transmit rate multiplier (e.g. 1.0)*/
 float fc_rx_loss; /*<  FC RX packet loss percentage (0.0 - 100.0)*/
 uint32_t fc_tx_err; /*<  FC Transmit error count*/
 uint32_t fc_bytes_rx; /*<  Total bytes received from Flight Controller*/
 uint32_t fc_bytes_tx; /*<  Total bytes transmitted to Flight Controller*/
 uint32_t fc_baudrate; /*<  Flight Controller UART baudrate, e.g. 921600*/
 float siyi_tx_rate; /*<  SIYI Transmit rate in Bytes/sec*/
 float siyi_rx_rate; /*<  SIYI Receive rate in Bytes/sec*/
 float siyi_tx_rate_max; /*<  SIYI Peak transmit rate in Bytes/sec*/
 float siyi_tx_rate_multi; /*<  SIYI Transmit rate multiplier (e.g. 1.0)*/
 float siyi_rx_loss; /*<  SIYI RX packet loss percentage (0.0 - 100.0)*/
 uint32_t siyi_tx_err; /*<  SIYI Transmit error count*/
 uint32_t siyi_bytes_rx; /*<  Total bytes received from SIYI link*/
 uint32_t siyi_bytes_tx; /*<  Total bytes transmitted to SIYI link*/
 uint32_t siyi_baudrate; /*<  SIYI telemetry link baudrate, e.g. 115200*/
 uint8_t fc_status; /*<  Flight Controller link status*/
 uint8_t siyi_status; /*<  SIYI Air Unit link status*/
 uint8_t transport_type; /*<  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP)*/
 char fc_port[16]; /*<  FC active serial device node, e.g. /dev/ttyAMA4*/
 char siyi_port[16]; /*<  SIYI active serial device node, e.g. /dev/ttyAMA0*/
 char available_ports[48]; /*<  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4*/
} mavlink_cc_telemetry_links_t;

#define MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN 155
#define MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN 155
#define MAVLINK_MSG_ID_42010_LEN 155
#define MAVLINK_MSG_ID_42010_MIN_LEN 155

#define MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC 101
#define MAVLINK_MSG_ID_42010_CRC 101

#define MAVLINK_MSG_CC_TELEMETRY_LINKS_FIELD_FC_PORT_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_LINKS_FIELD_SIYI_PORT_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_LINKS_FIELD_AVAILABLE_PORTS_LEN 48

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_LINKS { \
    42010, \
    "CC_TELEMETRY_LINKS", \
    24, \
    {  { "fc_tx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_cc_telemetry_links_t, fc_tx_rate) }, \
         { "fc_rx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_cc_telemetry_links_t, fc_rx_rate) }, \
         { "fc_tx_rate_max", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_cc_telemetry_links_t, fc_tx_rate_max) }, \
         { "fc_tx_rate_multi", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_cc_telemetry_links_t, fc_tx_rate_multi) }, \
         { "fc_rx_loss", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_cc_telemetry_links_t, fc_rx_loss) }, \
         { "fc_tx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_cc_telemetry_links_t, fc_tx_err) }, \
         { "fc_bytes_rx", NULL, MAVLINK_TYPE_UINT32_T, 0, 24, offsetof(mavlink_cc_telemetry_links_t, fc_bytes_rx) }, \
         { "fc_bytes_tx", NULL, MAVLINK_TYPE_UINT32_T, 0, 28, offsetof(mavlink_cc_telemetry_links_t, fc_bytes_tx) }, \
         { "fc_baudrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 32, offsetof(mavlink_cc_telemetry_links_t, fc_baudrate) }, \
         { "siyi_tx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 36, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_rate) }, \
         { "siyi_rx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 40, offsetof(mavlink_cc_telemetry_links_t, siyi_rx_rate) }, \
         { "siyi_tx_rate_max", NULL, MAVLINK_TYPE_FLOAT, 0, 44, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_rate_max) }, \
         { "siyi_tx_rate_multi", NULL, MAVLINK_TYPE_FLOAT, 0, 48, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_rate_multi) }, \
         { "siyi_rx_loss", NULL, MAVLINK_TYPE_FLOAT, 0, 52, offsetof(mavlink_cc_telemetry_links_t, siyi_rx_loss) }, \
         { "siyi_tx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 56, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_err) }, \
         { "siyi_bytes_rx", NULL, MAVLINK_TYPE_UINT32_T, 0, 60, offsetof(mavlink_cc_telemetry_links_t, siyi_bytes_rx) }, \
         { "siyi_bytes_tx", NULL, MAVLINK_TYPE_UINT32_T, 0, 64, offsetof(mavlink_cc_telemetry_links_t, siyi_bytes_tx) }, \
         { "siyi_baudrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 68, offsetof(mavlink_cc_telemetry_links_t, siyi_baudrate) }, \
         { "fc_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 72, offsetof(mavlink_cc_telemetry_links_t, fc_status) }, \
         { "siyi_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 73, offsetof(mavlink_cc_telemetry_links_t, siyi_status) }, \
         { "transport_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 74, offsetof(mavlink_cc_telemetry_links_t, transport_type) }, \
         { "fc_port", NULL, MAVLINK_TYPE_CHAR, 16, 75, offsetof(mavlink_cc_telemetry_links_t, fc_port) }, \
         { "siyi_port", NULL, MAVLINK_TYPE_CHAR, 16, 91, offsetof(mavlink_cc_telemetry_links_t, siyi_port) }, \
         { "available_ports", NULL, MAVLINK_TYPE_CHAR, 48, 107, offsetof(mavlink_cc_telemetry_links_t, available_ports) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_LINKS { \
    "CC_TELEMETRY_LINKS", \
    24, \
    {  { "fc_tx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_cc_telemetry_links_t, fc_tx_rate) }, \
         { "fc_rx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_cc_telemetry_links_t, fc_rx_rate) }, \
         { "fc_tx_rate_max", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_cc_telemetry_links_t, fc_tx_rate_max) }, \
         { "fc_tx_rate_multi", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_cc_telemetry_links_t, fc_tx_rate_multi) }, \
         { "fc_rx_loss", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_cc_telemetry_links_t, fc_rx_loss) }, \
         { "fc_tx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_cc_telemetry_links_t, fc_tx_err) }, \
         { "fc_bytes_rx", NULL, MAVLINK_TYPE_UINT32_T, 0, 24, offsetof(mavlink_cc_telemetry_links_t, fc_bytes_rx) }, \
         { "fc_bytes_tx", NULL, MAVLINK_TYPE_UINT32_T, 0, 28, offsetof(mavlink_cc_telemetry_links_t, fc_bytes_tx) }, \
         { "fc_baudrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 32, offsetof(mavlink_cc_telemetry_links_t, fc_baudrate) }, \
         { "siyi_tx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 36, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_rate) }, \
         { "siyi_rx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 40, offsetof(mavlink_cc_telemetry_links_t, siyi_rx_rate) }, \
         { "siyi_tx_rate_max", NULL, MAVLINK_TYPE_FLOAT, 0, 44, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_rate_max) }, \
         { "siyi_tx_rate_multi", NULL, MAVLINK_TYPE_FLOAT, 0, 48, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_rate_multi) }, \
         { "siyi_rx_loss", NULL, MAVLINK_TYPE_FLOAT, 0, 52, offsetof(mavlink_cc_telemetry_links_t, siyi_rx_loss) }, \
         { "siyi_tx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 56, offsetof(mavlink_cc_telemetry_links_t, siyi_tx_err) }, \
         { "siyi_bytes_rx", NULL, MAVLINK_TYPE_UINT32_T, 0, 60, offsetof(mavlink_cc_telemetry_links_t, siyi_bytes_rx) }, \
         { "siyi_bytes_tx", NULL, MAVLINK_TYPE_UINT32_T, 0, 64, offsetof(mavlink_cc_telemetry_links_t, siyi_bytes_tx) }, \
         { "siyi_baudrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 68, offsetof(mavlink_cc_telemetry_links_t, siyi_baudrate) }, \
         { "fc_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 72, offsetof(mavlink_cc_telemetry_links_t, fc_status) }, \
         { "siyi_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 73, offsetof(mavlink_cc_telemetry_links_t, siyi_status) }, \
         { "transport_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 74, offsetof(mavlink_cc_telemetry_links_t, transport_type) }, \
         { "fc_port", NULL, MAVLINK_TYPE_CHAR, 16, 75, offsetof(mavlink_cc_telemetry_links_t, fc_port) }, \
         { "siyi_port", NULL, MAVLINK_TYPE_CHAR, 16, 91, offsetof(mavlink_cc_telemetry_links_t, siyi_port) }, \
         { "available_ports", NULL, MAVLINK_TYPE_CHAR, 48, 107, offsetof(mavlink_cc_telemetry_links_t, available_ports) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_telemetry_links message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param fc_tx_rate  FC Transmit rate in Bytes/sec
 * @param fc_rx_rate  FC Receive rate in Bytes/sec
 * @param fc_tx_rate_max  FC Peak transmit rate in Bytes/sec
 * @param fc_tx_rate_multi  FC Transmit rate multiplier (e.g. 1.0)
 * @param fc_rx_loss  FC RX packet loss percentage (0.0 - 100.0)
 * @param fc_tx_err  FC Transmit error count
 * @param fc_bytes_rx  Total bytes received from Flight Controller
 * @param fc_bytes_tx  Total bytes transmitted to Flight Controller
 * @param fc_baudrate  Flight Controller UART baudrate, e.g. 921600
 * @param siyi_tx_rate  SIYI Transmit rate in Bytes/sec
 * @param siyi_rx_rate  SIYI Receive rate in Bytes/sec
 * @param siyi_tx_rate_max  SIYI Peak transmit rate in Bytes/sec
 * @param siyi_tx_rate_multi  SIYI Transmit rate multiplier (e.g. 1.0)
 * @param siyi_rx_loss  SIYI RX packet loss percentage (0.0 - 100.0)
 * @param siyi_tx_err  SIYI Transmit error count
 * @param siyi_bytes_rx  Total bytes received from SIYI link
 * @param siyi_bytes_tx  Total bytes transmitted to SIYI link
 * @param siyi_baudrate  SIYI telemetry link baudrate, e.g. 115200
 * @param fc_status  Flight Controller link status
 * @param siyi_status  SIYI Air Unit link status
 * @param transport_type  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP)
 * @param fc_port  FC active serial device node, e.g. /dev/ttyAMA4
 * @param siyi_port  SIYI active serial device node, e.g. /dev/ttyAMA0
 * @param available_ports  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               float fc_tx_rate, float fc_rx_rate, float fc_tx_rate_max, float fc_tx_rate_multi, float fc_rx_loss, uint32_t fc_tx_err, uint32_t fc_bytes_rx, uint32_t fc_bytes_tx, uint32_t fc_baudrate, float siyi_tx_rate, float siyi_rx_rate, float siyi_tx_rate_max, float siyi_tx_rate_multi, float siyi_rx_loss, uint32_t siyi_tx_err, uint32_t siyi_bytes_rx, uint32_t siyi_bytes_tx, uint32_t siyi_baudrate, uint8_t fc_status, uint8_t siyi_status, uint8_t transport_type, const char *fc_port, const char *siyi_port, const char *available_ports)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN];
    _mav_put_float(buf, 0, fc_tx_rate);
    _mav_put_float(buf, 4, fc_rx_rate);
    _mav_put_float(buf, 8, fc_tx_rate_max);
    _mav_put_float(buf, 12, fc_tx_rate_multi);
    _mav_put_float(buf, 16, fc_rx_loss);
    _mav_put_uint32_t(buf, 20, fc_tx_err);
    _mav_put_uint32_t(buf, 24, fc_bytes_rx);
    _mav_put_uint32_t(buf, 28, fc_bytes_tx);
    _mav_put_uint32_t(buf, 32, fc_baudrate);
    _mav_put_float(buf, 36, siyi_tx_rate);
    _mav_put_float(buf, 40, siyi_rx_rate);
    _mav_put_float(buf, 44, siyi_tx_rate_max);
    _mav_put_float(buf, 48, siyi_tx_rate_multi);
    _mav_put_float(buf, 52, siyi_rx_loss);
    _mav_put_uint32_t(buf, 56, siyi_tx_err);
    _mav_put_uint32_t(buf, 60, siyi_bytes_rx);
    _mav_put_uint32_t(buf, 64, siyi_bytes_tx);
    _mav_put_uint32_t(buf, 68, siyi_baudrate);
    _mav_put_uint8_t(buf, 72, fc_status);
    _mav_put_uint8_t(buf, 73, siyi_status);
    _mav_put_uint8_t(buf, 74, transport_type);
    _mav_put_char_array(buf, 75, fc_port, 16);
    _mav_put_char_array(buf, 91, siyi_port, 16);
    _mav_put_char_array(buf, 107, available_ports, 48);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#else
    mavlink_cc_telemetry_links_t packet;
    packet.fc_tx_rate = fc_tx_rate;
    packet.fc_rx_rate = fc_rx_rate;
    packet.fc_tx_rate_max = fc_tx_rate_max;
    packet.fc_tx_rate_multi = fc_tx_rate_multi;
    packet.fc_rx_loss = fc_rx_loss;
    packet.fc_tx_err = fc_tx_err;
    packet.fc_bytes_rx = fc_bytes_rx;
    packet.fc_bytes_tx = fc_bytes_tx;
    packet.fc_baudrate = fc_baudrate;
    packet.siyi_tx_rate = siyi_tx_rate;
    packet.siyi_rx_rate = siyi_rx_rate;
    packet.siyi_tx_rate_max = siyi_tx_rate_max;
    packet.siyi_tx_rate_multi = siyi_tx_rate_multi;
    packet.siyi_rx_loss = siyi_rx_loss;
    packet.siyi_tx_err = siyi_tx_err;
    packet.siyi_bytes_rx = siyi_bytes_rx;
    packet.siyi_bytes_tx = siyi_bytes_tx;
    packet.siyi_baudrate = siyi_baudrate;
    packet.fc_status = fc_status;
    packet.siyi_status = siyi_status;
    packet.transport_type = transport_type;
    mav_array_memcpy(packet.fc_port, fc_port, sizeof(char)*16);
    mav_array_memcpy(packet.siyi_port, siyi_port, sizeof(char)*16);
    mav_array_memcpy(packet.available_ports, available_ports, sizeof(char)*48);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_LINKS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
}

/**
 * @brief Pack a cc_telemetry_links message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param fc_tx_rate  FC Transmit rate in Bytes/sec
 * @param fc_rx_rate  FC Receive rate in Bytes/sec
 * @param fc_tx_rate_max  FC Peak transmit rate in Bytes/sec
 * @param fc_tx_rate_multi  FC Transmit rate multiplier (e.g. 1.0)
 * @param fc_rx_loss  FC RX packet loss percentage (0.0 - 100.0)
 * @param fc_tx_err  FC Transmit error count
 * @param fc_bytes_rx  Total bytes received from Flight Controller
 * @param fc_bytes_tx  Total bytes transmitted to Flight Controller
 * @param fc_baudrate  Flight Controller UART baudrate, e.g. 921600
 * @param siyi_tx_rate  SIYI Transmit rate in Bytes/sec
 * @param siyi_rx_rate  SIYI Receive rate in Bytes/sec
 * @param siyi_tx_rate_max  SIYI Peak transmit rate in Bytes/sec
 * @param siyi_tx_rate_multi  SIYI Transmit rate multiplier (e.g. 1.0)
 * @param siyi_rx_loss  SIYI RX packet loss percentage (0.0 - 100.0)
 * @param siyi_tx_err  SIYI Transmit error count
 * @param siyi_bytes_rx  Total bytes received from SIYI link
 * @param siyi_bytes_tx  Total bytes transmitted to SIYI link
 * @param siyi_baudrate  SIYI telemetry link baudrate, e.g. 115200
 * @param fc_status  Flight Controller link status
 * @param siyi_status  SIYI Air Unit link status
 * @param transport_type  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP)
 * @param fc_port  FC active serial device node, e.g. /dev/ttyAMA4
 * @param siyi_port  SIYI active serial device node, e.g. /dev/ttyAMA0
 * @param available_ports  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               float fc_tx_rate, float fc_rx_rate, float fc_tx_rate_max, float fc_tx_rate_multi, float fc_rx_loss, uint32_t fc_tx_err, uint32_t fc_bytes_rx, uint32_t fc_bytes_tx, uint32_t fc_baudrate, float siyi_tx_rate, float siyi_rx_rate, float siyi_tx_rate_max, float siyi_tx_rate_multi, float siyi_rx_loss, uint32_t siyi_tx_err, uint32_t siyi_bytes_rx, uint32_t siyi_bytes_tx, uint32_t siyi_baudrate, uint8_t fc_status, uint8_t siyi_status, uint8_t transport_type, const char *fc_port, const char *siyi_port, const char *available_ports)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN];
    _mav_put_float(buf, 0, fc_tx_rate);
    _mav_put_float(buf, 4, fc_rx_rate);
    _mav_put_float(buf, 8, fc_tx_rate_max);
    _mav_put_float(buf, 12, fc_tx_rate_multi);
    _mav_put_float(buf, 16, fc_rx_loss);
    _mav_put_uint32_t(buf, 20, fc_tx_err);
    _mav_put_uint32_t(buf, 24, fc_bytes_rx);
    _mav_put_uint32_t(buf, 28, fc_bytes_tx);
    _mav_put_uint32_t(buf, 32, fc_baudrate);
    _mav_put_float(buf, 36, siyi_tx_rate);
    _mav_put_float(buf, 40, siyi_rx_rate);
    _mav_put_float(buf, 44, siyi_tx_rate_max);
    _mav_put_float(buf, 48, siyi_tx_rate_multi);
    _mav_put_float(buf, 52, siyi_rx_loss);
    _mav_put_uint32_t(buf, 56, siyi_tx_err);
    _mav_put_uint32_t(buf, 60, siyi_bytes_rx);
    _mav_put_uint32_t(buf, 64, siyi_bytes_tx);
    _mav_put_uint32_t(buf, 68, siyi_baudrate);
    _mav_put_uint8_t(buf, 72, fc_status);
    _mav_put_uint8_t(buf, 73, siyi_status);
    _mav_put_uint8_t(buf, 74, transport_type);
    _mav_put_char_array(buf, 75, fc_port, 16);
    _mav_put_char_array(buf, 91, siyi_port, 16);
    _mav_put_char_array(buf, 107, available_ports, 48);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#else
    mavlink_cc_telemetry_links_t packet;
    packet.fc_tx_rate = fc_tx_rate;
    packet.fc_rx_rate = fc_rx_rate;
    packet.fc_tx_rate_max = fc_tx_rate_max;
    packet.fc_tx_rate_multi = fc_tx_rate_multi;
    packet.fc_rx_loss = fc_rx_loss;
    packet.fc_tx_err = fc_tx_err;
    packet.fc_bytes_rx = fc_bytes_rx;
    packet.fc_bytes_tx = fc_bytes_tx;
    packet.fc_baudrate = fc_baudrate;
    packet.siyi_tx_rate = siyi_tx_rate;
    packet.siyi_rx_rate = siyi_rx_rate;
    packet.siyi_tx_rate_max = siyi_tx_rate_max;
    packet.siyi_tx_rate_multi = siyi_tx_rate_multi;
    packet.siyi_rx_loss = siyi_rx_loss;
    packet.siyi_tx_err = siyi_tx_err;
    packet.siyi_bytes_rx = siyi_bytes_rx;
    packet.siyi_bytes_tx = siyi_bytes_tx;
    packet.siyi_baudrate = siyi_baudrate;
    packet.fc_status = fc_status;
    packet.siyi_status = siyi_status;
    packet.transport_type = transport_type;
    mav_array_memcpy(packet.fc_port, fc_port, sizeof(char)*16);
    mav_array_memcpy(packet.siyi_port, siyi_port, sizeof(char)*16);
    mav_array_memcpy(packet.available_ports, available_ports, sizeof(char)*48);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_LINKS;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#endif
}

/**
 * @brief Pack a cc_telemetry_links message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param fc_tx_rate  FC Transmit rate in Bytes/sec
 * @param fc_rx_rate  FC Receive rate in Bytes/sec
 * @param fc_tx_rate_max  FC Peak transmit rate in Bytes/sec
 * @param fc_tx_rate_multi  FC Transmit rate multiplier (e.g. 1.0)
 * @param fc_rx_loss  FC RX packet loss percentage (0.0 - 100.0)
 * @param fc_tx_err  FC Transmit error count
 * @param fc_bytes_rx  Total bytes received from Flight Controller
 * @param fc_bytes_tx  Total bytes transmitted to Flight Controller
 * @param fc_baudrate  Flight Controller UART baudrate, e.g. 921600
 * @param siyi_tx_rate  SIYI Transmit rate in Bytes/sec
 * @param siyi_rx_rate  SIYI Receive rate in Bytes/sec
 * @param siyi_tx_rate_max  SIYI Peak transmit rate in Bytes/sec
 * @param siyi_tx_rate_multi  SIYI Transmit rate multiplier (e.g. 1.0)
 * @param siyi_rx_loss  SIYI RX packet loss percentage (0.0 - 100.0)
 * @param siyi_tx_err  SIYI Transmit error count
 * @param siyi_bytes_rx  Total bytes received from SIYI link
 * @param siyi_bytes_tx  Total bytes transmitted to SIYI link
 * @param siyi_baudrate  SIYI telemetry link baudrate, e.g. 115200
 * @param fc_status  Flight Controller link status
 * @param siyi_status  SIYI Air Unit link status
 * @param transport_type  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP)
 * @param fc_port  FC active serial device node, e.g. /dev/ttyAMA4
 * @param siyi_port  SIYI active serial device node, e.g. /dev/ttyAMA0
 * @param available_ports  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   float fc_tx_rate,float fc_rx_rate,float fc_tx_rate_max,float fc_tx_rate_multi,float fc_rx_loss,uint32_t fc_tx_err,uint32_t fc_bytes_rx,uint32_t fc_bytes_tx,uint32_t fc_baudrate,float siyi_tx_rate,float siyi_rx_rate,float siyi_tx_rate_max,float siyi_tx_rate_multi,float siyi_rx_loss,uint32_t siyi_tx_err,uint32_t siyi_bytes_rx,uint32_t siyi_bytes_tx,uint32_t siyi_baudrate,uint8_t fc_status,uint8_t siyi_status,uint8_t transport_type,const char *fc_port,const char *siyi_port,const char *available_ports)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN];
    _mav_put_float(buf, 0, fc_tx_rate);
    _mav_put_float(buf, 4, fc_rx_rate);
    _mav_put_float(buf, 8, fc_tx_rate_max);
    _mav_put_float(buf, 12, fc_tx_rate_multi);
    _mav_put_float(buf, 16, fc_rx_loss);
    _mav_put_uint32_t(buf, 20, fc_tx_err);
    _mav_put_uint32_t(buf, 24, fc_bytes_rx);
    _mav_put_uint32_t(buf, 28, fc_bytes_tx);
    _mav_put_uint32_t(buf, 32, fc_baudrate);
    _mav_put_float(buf, 36, siyi_tx_rate);
    _mav_put_float(buf, 40, siyi_rx_rate);
    _mav_put_float(buf, 44, siyi_tx_rate_max);
    _mav_put_float(buf, 48, siyi_tx_rate_multi);
    _mav_put_float(buf, 52, siyi_rx_loss);
    _mav_put_uint32_t(buf, 56, siyi_tx_err);
    _mav_put_uint32_t(buf, 60, siyi_bytes_rx);
    _mav_put_uint32_t(buf, 64, siyi_bytes_tx);
    _mav_put_uint32_t(buf, 68, siyi_baudrate);
    _mav_put_uint8_t(buf, 72, fc_status);
    _mav_put_uint8_t(buf, 73, siyi_status);
    _mav_put_uint8_t(buf, 74, transport_type);
    _mav_put_char_array(buf, 75, fc_port, 16);
    _mav_put_char_array(buf, 91, siyi_port, 16);
    _mav_put_char_array(buf, 107, available_ports, 48);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#else
    mavlink_cc_telemetry_links_t packet;
    packet.fc_tx_rate = fc_tx_rate;
    packet.fc_rx_rate = fc_rx_rate;
    packet.fc_tx_rate_max = fc_tx_rate_max;
    packet.fc_tx_rate_multi = fc_tx_rate_multi;
    packet.fc_rx_loss = fc_rx_loss;
    packet.fc_tx_err = fc_tx_err;
    packet.fc_bytes_rx = fc_bytes_rx;
    packet.fc_bytes_tx = fc_bytes_tx;
    packet.fc_baudrate = fc_baudrate;
    packet.siyi_tx_rate = siyi_tx_rate;
    packet.siyi_rx_rate = siyi_rx_rate;
    packet.siyi_tx_rate_max = siyi_tx_rate_max;
    packet.siyi_tx_rate_multi = siyi_tx_rate_multi;
    packet.siyi_rx_loss = siyi_rx_loss;
    packet.siyi_tx_err = siyi_tx_err;
    packet.siyi_bytes_rx = siyi_bytes_rx;
    packet.siyi_bytes_tx = siyi_bytes_tx;
    packet.siyi_baudrate = siyi_baudrate;
    packet.fc_status = fc_status;
    packet.siyi_status = siyi_status;
    packet.transport_type = transport_type;
    mav_array_memcpy(packet.fc_port, fc_port, sizeof(char)*16);
    mav_array_memcpy(packet.siyi_port, siyi_port, sizeof(char)*16);
    mav_array_memcpy(packet.available_ports, available_ports, sizeof(char)*48);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_LINKS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
}

/**
 * @brief Encode a cc_telemetry_links struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_links C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_telemetry_links_t* cc_telemetry_links)
{
    return mavlink_msg_cc_telemetry_links_pack(system_id, component_id, msg, cc_telemetry_links->fc_tx_rate, cc_telemetry_links->fc_rx_rate, cc_telemetry_links->fc_tx_rate_max, cc_telemetry_links->fc_tx_rate_multi, cc_telemetry_links->fc_rx_loss, cc_telemetry_links->fc_tx_err, cc_telemetry_links->fc_bytes_rx, cc_telemetry_links->fc_bytes_tx, cc_telemetry_links->fc_baudrate, cc_telemetry_links->siyi_tx_rate, cc_telemetry_links->siyi_rx_rate, cc_telemetry_links->siyi_tx_rate_max, cc_telemetry_links->siyi_tx_rate_multi, cc_telemetry_links->siyi_rx_loss, cc_telemetry_links->siyi_tx_err, cc_telemetry_links->siyi_bytes_rx, cc_telemetry_links->siyi_bytes_tx, cc_telemetry_links->siyi_baudrate, cc_telemetry_links->fc_status, cc_telemetry_links->siyi_status, cc_telemetry_links->transport_type, cc_telemetry_links->fc_port, cc_telemetry_links->siyi_port, cc_telemetry_links->available_ports);
}

/**
 * @brief Encode a cc_telemetry_links struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_links C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_telemetry_links_t* cc_telemetry_links)
{
    return mavlink_msg_cc_telemetry_links_pack_chan(system_id, component_id, chan, msg, cc_telemetry_links->fc_tx_rate, cc_telemetry_links->fc_rx_rate, cc_telemetry_links->fc_tx_rate_max, cc_telemetry_links->fc_tx_rate_multi, cc_telemetry_links->fc_rx_loss, cc_telemetry_links->fc_tx_err, cc_telemetry_links->fc_bytes_rx, cc_telemetry_links->fc_bytes_tx, cc_telemetry_links->fc_baudrate, cc_telemetry_links->siyi_tx_rate, cc_telemetry_links->siyi_rx_rate, cc_telemetry_links->siyi_tx_rate_max, cc_telemetry_links->siyi_tx_rate_multi, cc_telemetry_links->siyi_rx_loss, cc_telemetry_links->siyi_tx_err, cc_telemetry_links->siyi_bytes_rx, cc_telemetry_links->siyi_bytes_tx, cc_telemetry_links->siyi_baudrate, cc_telemetry_links->fc_status, cc_telemetry_links->siyi_status, cc_telemetry_links->transport_type, cc_telemetry_links->fc_port, cc_telemetry_links->siyi_port, cc_telemetry_links->available_ports);
}

/**
 * @brief Encode a cc_telemetry_links struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_links C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_telemetry_links_t* cc_telemetry_links)
{
    return mavlink_msg_cc_telemetry_links_pack_status(system_id, component_id, _status, msg,  cc_telemetry_links->fc_tx_rate, cc_telemetry_links->fc_rx_rate, cc_telemetry_links->fc_tx_rate_max, cc_telemetry_links->fc_tx_rate_multi, cc_telemetry_links->fc_rx_loss, cc_telemetry_links->fc_tx_err, cc_telemetry_links->fc_bytes_rx, cc_telemetry_links->fc_bytes_tx, cc_telemetry_links->fc_baudrate, cc_telemetry_links->siyi_tx_rate, cc_telemetry_links->siyi_rx_rate, cc_telemetry_links->siyi_tx_rate_max, cc_telemetry_links->siyi_tx_rate_multi, cc_telemetry_links->siyi_rx_loss, cc_telemetry_links->siyi_tx_err, cc_telemetry_links->siyi_bytes_rx, cc_telemetry_links->siyi_bytes_tx, cc_telemetry_links->siyi_baudrate, cc_telemetry_links->fc_status, cc_telemetry_links->siyi_status, cc_telemetry_links->transport_type, cc_telemetry_links->fc_port, cc_telemetry_links->siyi_port, cc_telemetry_links->available_ports);
}

/**
 * @brief Send a cc_telemetry_links message
 * @param chan MAVLink channel to send the message
 *
 * @param fc_tx_rate  FC Transmit rate in Bytes/sec
 * @param fc_rx_rate  FC Receive rate in Bytes/sec
 * @param fc_tx_rate_max  FC Peak transmit rate in Bytes/sec
 * @param fc_tx_rate_multi  FC Transmit rate multiplier (e.g. 1.0)
 * @param fc_rx_loss  FC RX packet loss percentage (0.0 - 100.0)
 * @param fc_tx_err  FC Transmit error count
 * @param fc_bytes_rx  Total bytes received from Flight Controller
 * @param fc_bytes_tx  Total bytes transmitted to Flight Controller
 * @param fc_baudrate  Flight Controller UART baudrate, e.g. 921600
 * @param siyi_tx_rate  SIYI Transmit rate in Bytes/sec
 * @param siyi_rx_rate  SIYI Receive rate in Bytes/sec
 * @param siyi_tx_rate_max  SIYI Peak transmit rate in Bytes/sec
 * @param siyi_tx_rate_multi  SIYI Transmit rate multiplier (e.g. 1.0)
 * @param siyi_rx_loss  SIYI RX packet loss percentage (0.0 - 100.0)
 * @param siyi_tx_err  SIYI Transmit error count
 * @param siyi_bytes_rx  Total bytes received from SIYI link
 * @param siyi_bytes_tx  Total bytes transmitted to SIYI link
 * @param siyi_baudrate  SIYI telemetry link baudrate, e.g. 115200
 * @param fc_status  Flight Controller link status
 * @param siyi_status  SIYI Air Unit link status
 * @param transport_type  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP)
 * @param fc_port  FC active serial device node, e.g. /dev/ttyAMA4
 * @param siyi_port  SIYI active serial device node, e.g. /dev/ttyAMA0
 * @param available_ports  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_telemetry_links_send(mavlink_channel_t chan, float fc_tx_rate, float fc_rx_rate, float fc_tx_rate_max, float fc_tx_rate_multi, float fc_rx_loss, uint32_t fc_tx_err, uint32_t fc_bytes_rx, uint32_t fc_bytes_tx, uint32_t fc_baudrate, float siyi_tx_rate, float siyi_rx_rate, float siyi_tx_rate_max, float siyi_tx_rate_multi, float siyi_rx_loss, uint32_t siyi_tx_err, uint32_t siyi_bytes_rx, uint32_t siyi_bytes_tx, uint32_t siyi_baudrate, uint8_t fc_status, uint8_t siyi_status, uint8_t transport_type, const char *fc_port, const char *siyi_port, const char *available_ports)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN];
    _mav_put_float(buf, 0, fc_tx_rate);
    _mav_put_float(buf, 4, fc_rx_rate);
    _mav_put_float(buf, 8, fc_tx_rate_max);
    _mav_put_float(buf, 12, fc_tx_rate_multi);
    _mav_put_float(buf, 16, fc_rx_loss);
    _mav_put_uint32_t(buf, 20, fc_tx_err);
    _mav_put_uint32_t(buf, 24, fc_bytes_rx);
    _mav_put_uint32_t(buf, 28, fc_bytes_tx);
    _mav_put_uint32_t(buf, 32, fc_baudrate);
    _mav_put_float(buf, 36, siyi_tx_rate);
    _mav_put_float(buf, 40, siyi_rx_rate);
    _mav_put_float(buf, 44, siyi_tx_rate_max);
    _mav_put_float(buf, 48, siyi_tx_rate_multi);
    _mav_put_float(buf, 52, siyi_rx_loss);
    _mav_put_uint32_t(buf, 56, siyi_tx_err);
    _mav_put_uint32_t(buf, 60, siyi_bytes_rx);
    _mav_put_uint32_t(buf, 64, siyi_bytes_tx);
    _mav_put_uint32_t(buf, 68, siyi_baudrate);
    _mav_put_uint8_t(buf, 72, fc_status);
    _mav_put_uint8_t(buf, 73, siyi_status);
    _mav_put_uint8_t(buf, 74, transport_type);
    _mav_put_char_array(buf, 75, fc_port, 16);
    _mav_put_char_array(buf, 91, siyi_port, 16);
    _mav_put_char_array(buf, 107, available_ports, 48);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS, buf, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
#else
    mavlink_cc_telemetry_links_t packet;
    packet.fc_tx_rate = fc_tx_rate;
    packet.fc_rx_rate = fc_rx_rate;
    packet.fc_tx_rate_max = fc_tx_rate_max;
    packet.fc_tx_rate_multi = fc_tx_rate_multi;
    packet.fc_rx_loss = fc_rx_loss;
    packet.fc_tx_err = fc_tx_err;
    packet.fc_bytes_rx = fc_bytes_rx;
    packet.fc_bytes_tx = fc_bytes_tx;
    packet.fc_baudrate = fc_baudrate;
    packet.siyi_tx_rate = siyi_tx_rate;
    packet.siyi_rx_rate = siyi_rx_rate;
    packet.siyi_tx_rate_max = siyi_tx_rate_max;
    packet.siyi_tx_rate_multi = siyi_tx_rate_multi;
    packet.siyi_rx_loss = siyi_rx_loss;
    packet.siyi_tx_err = siyi_tx_err;
    packet.siyi_bytes_rx = siyi_bytes_rx;
    packet.siyi_bytes_tx = siyi_bytes_tx;
    packet.siyi_baudrate = siyi_baudrate;
    packet.fc_status = fc_status;
    packet.siyi_status = siyi_status;
    packet.transport_type = transport_type;
    mav_array_memcpy(packet.fc_port, fc_port, sizeof(char)*16);
    mav_array_memcpy(packet.siyi_port, siyi_port, sizeof(char)*16);
    mav_array_memcpy(packet.available_ports, available_ports, sizeof(char)*48);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS, (const char *)&packet, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
#endif
}

/**
 * @brief Send a cc_telemetry_links message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_telemetry_links_send_struct(mavlink_channel_t chan, const mavlink_cc_telemetry_links_t* cc_telemetry_links)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_telemetry_links_send(chan, cc_telemetry_links->fc_tx_rate, cc_telemetry_links->fc_rx_rate, cc_telemetry_links->fc_tx_rate_max, cc_telemetry_links->fc_tx_rate_multi, cc_telemetry_links->fc_rx_loss, cc_telemetry_links->fc_tx_err, cc_telemetry_links->fc_bytes_rx, cc_telemetry_links->fc_bytes_tx, cc_telemetry_links->fc_baudrate, cc_telemetry_links->siyi_tx_rate, cc_telemetry_links->siyi_rx_rate, cc_telemetry_links->siyi_tx_rate_max, cc_telemetry_links->siyi_tx_rate_multi, cc_telemetry_links->siyi_rx_loss, cc_telemetry_links->siyi_tx_err, cc_telemetry_links->siyi_bytes_rx, cc_telemetry_links->siyi_bytes_tx, cc_telemetry_links->siyi_baudrate, cc_telemetry_links->fc_status, cc_telemetry_links->siyi_status, cc_telemetry_links->transport_type, cc_telemetry_links->fc_port, cc_telemetry_links->siyi_port, cc_telemetry_links->available_ports);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS, (const char *)cc_telemetry_links, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_telemetry_links_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  float fc_tx_rate, float fc_rx_rate, float fc_tx_rate_max, float fc_tx_rate_multi, float fc_rx_loss, uint32_t fc_tx_err, uint32_t fc_bytes_rx, uint32_t fc_bytes_tx, uint32_t fc_baudrate, float siyi_tx_rate, float siyi_rx_rate, float siyi_tx_rate_max, float siyi_tx_rate_multi, float siyi_rx_loss, uint32_t siyi_tx_err, uint32_t siyi_bytes_rx, uint32_t siyi_bytes_tx, uint32_t siyi_baudrate, uint8_t fc_status, uint8_t siyi_status, uint8_t transport_type, const char *fc_port, const char *siyi_port, const char *available_ports)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, fc_tx_rate);
    _mav_put_float(buf, 4, fc_rx_rate);
    _mav_put_float(buf, 8, fc_tx_rate_max);
    _mav_put_float(buf, 12, fc_tx_rate_multi);
    _mav_put_float(buf, 16, fc_rx_loss);
    _mav_put_uint32_t(buf, 20, fc_tx_err);
    _mav_put_uint32_t(buf, 24, fc_bytes_rx);
    _mav_put_uint32_t(buf, 28, fc_bytes_tx);
    _mav_put_uint32_t(buf, 32, fc_baudrate);
    _mav_put_float(buf, 36, siyi_tx_rate);
    _mav_put_float(buf, 40, siyi_rx_rate);
    _mav_put_float(buf, 44, siyi_tx_rate_max);
    _mav_put_float(buf, 48, siyi_tx_rate_multi);
    _mav_put_float(buf, 52, siyi_rx_loss);
    _mav_put_uint32_t(buf, 56, siyi_tx_err);
    _mav_put_uint32_t(buf, 60, siyi_bytes_rx);
    _mav_put_uint32_t(buf, 64, siyi_bytes_tx);
    _mav_put_uint32_t(buf, 68, siyi_baudrate);
    _mav_put_uint8_t(buf, 72, fc_status);
    _mav_put_uint8_t(buf, 73, siyi_status);
    _mav_put_uint8_t(buf, 74, transport_type);
    _mav_put_char_array(buf, 75, fc_port, 16);
    _mav_put_char_array(buf, 91, siyi_port, 16);
    _mav_put_char_array(buf, 107, available_ports, 48);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS, buf, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
#else
    mavlink_cc_telemetry_links_t *packet = (mavlink_cc_telemetry_links_t *)msgbuf;
    packet->fc_tx_rate = fc_tx_rate;
    packet->fc_rx_rate = fc_rx_rate;
    packet->fc_tx_rate_max = fc_tx_rate_max;
    packet->fc_tx_rate_multi = fc_tx_rate_multi;
    packet->fc_rx_loss = fc_rx_loss;
    packet->fc_tx_err = fc_tx_err;
    packet->fc_bytes_rx = fc_bytes_rx;
    packet->fc_bytes_tx = fc_bytes_tx;
    packet->fc_baudrate = fc_baudrate;
    packet->siyi_tx_rate = siyi_tx_rate;
    packet->siyi_rx_rate = siyi_rx_rate;
    packet->siyi_tx_rate_max = siyi_tx_rate_max;
    packet->siyi_tx_rate_multi = siyi_tx_rate_multi;
    packet->siyi_rx_loss = siyi_rx_loss;
    packet->siyi_tx_err = siyi_tx_err;
    packet->siyi_bytes_rx = siyi_bytes_rx;
    packet->siyi_bytes_tx = siyi_bytes_tx;
    packet->siyi_baudrate = siyi_baudrate;
    packet->fc_status = fc_status;
    packet->siyi_status = siyi_status;
    packet->transport_type = transport_type;
    mav_array_memcpy(packet->fc_port, fc_port, sizeof(char)*16);
    mav_array_memcpy(packet->siyi_port, siyi_port, sizeof(char)*16);
    mav_array_memcpy(packet->available_ports, available_ports, sizeof(char)*48);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS, (const char *)packet, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_TELEMETRY_LINKS UNPACKING


/**
 * @brief Get field fc_tx_rate from cc_telemetry_links message
 *
 * @return  FC Transmit rate in Bytes/sec
 */
static inline float mavlink_msg_cc_telemetry_links_get_fc_tx_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field fc_rx_rate from cc_telemetry_links message
 *
 * @return  FC Receive rate in Bytes/sec
 */
static inline float mavlink_msg_cc_telemetry_links_get_fc_rx_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field fc_tx_rate_max from cc_telemetry_links message
 *
 * @return  FC Peak transmit rate in Bytes/sec
 */
static inline float mavlink_msg_cc_telemetry_links_get_fc_tx_rate_max(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field fc_tx_rate_multi from cc_telemetry_links message
 *
 * @return  FC Transmit rate multiplier (e.g. 1.0)
 */
static inline float mavlink_msg_cc_telemetry_links_get_fc_tx_rate_multi(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field fc_rx_loss from cc_telemetry_links message
 *
 * @return  FC RX packet loss percentage (0.0 - 100.0)
 */
static inline float mavlink_msg_cc_telemetry_links_get_fc_rx_loss(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Get field fc_tx_err from cc_telemetry_links message
 *
 * @return  FC Transmit error count
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_fc_tx_err(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  20);
}

/**
 * @brief Get field fc_bytes_rx from cc_telemetry_links message
 *
 * @return  Total bytes received from Flight Controller
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_fc_bytes_rx(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  24);
}

/**
 * @brief Get field fc_bytes_tx from cc_telemetry_links message
 *
 * @return  Total bytes transmitted to Flight Controller
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_fc_bytes_tx(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  28);
}

/**
 * @brief Get field fc_baudrate from cc_telemetry_links message
 *
 * @return  Flight Controller UART baudrate, e.g. 921600
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_fc_baudrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  32);
}

/**
 * @brief Get field siyi_tx_rate from cc_telemetry_links message
 *
 * @return  SIYI Transmit rate in Bytes/sec
 */
static inline float mavlink_msg_cc_telemetry_links_get_siyi_tx_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  36);
}

/**
 * @brief Get field siyi_rx_rate from cc_telemetry_links message
 *
 * @return  SIYI Receive rate in Bytes/sec
 */
static inline float mavlink_msg_cc_telemetry_links_get_siyi_rx_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  40);
}

/**
 * @brief Get field siyi_tx_rate_max from cc_telemetry_links message
 *
 * @return  SIYI Peak transmit rate in Bytes/sec
 */
static inline float mavlink_msg_cc_telemetry_links_get_siyi_tx_rate_max(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  44);
}

/**
 * @brief Get field siyi_tx_rate_multi from cc_telemetry_links message
 *
 * @return  SIYI Transmit rate multiplier (e.g. 1.0)
 */
static inline float mavlink_msg_cc_telemetry_links_get_siyi_tx_rate_multi(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  48);
}

/**
 * @brief Get field siyi_rx_loss from cc_telemetry_links message
 *
 * @return  SIYI RX packet loss percentage (0.0 - 100.0)
 */
static inline float mavlink_msg_cc_telemetry_links_get_siyi_rx_loss(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  52);
}

/**
 * @brief Get field siyi_tx_err from cc_telemetry_links message
 *
 * @return  SIYI Transmit error count
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_siyi_tx_err(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  56);
}

/**
 * @brief Get field siyi_bytes_rx from cc_telemetry_links message
 *
 * @return  Total bytes received from SIYI link
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_siyi_bytes_rx(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  60);
}

/**
 * @brief Get field siyi_bytes_tx from cc_telemetry_links message
 *
 * @return  Total bytes transmitted to SIYI link
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_siyi_bytes_tx(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  64);
}

/**
 * @brief Get field siyi_baudrate from cc_telemetry_links message
 *
 * @return  SIYI telemetry link baudrate, e.g. 115200
 */
static inline uint32_t mavlink_msg_cc_telemetry_links_get_siyi_baudrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  68);
}

/**
 * @brief Get field fc_status from cc_telemetry_links message
 *
 * @return  Flight Controller link status
 */
static inline uint8_t mavlink_msg_cc_telemetry_links_get_fc_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  72);
}

/**
 * @brief Get field siyi_status from cc_telemetry_links message
 *
 * @return  SIYI Air Unit link status
 */
static inline uint8_t mavlink_msg_cc_telemetry_links_get_siyi_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  73);
}

/**
 * @brief Get field transport_type from cc_telemetry_links message
 *
 * @return  Transport protocol (1: Serial/UART, 2: UDP, 3: TCP)
 */
static inline uint8_t mavlink_msg_cc_telemetry_links_get_transport_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  74);
}

/**
 * @brief Get field fc_port from cc_telemetry_links message
 *
 * @return  FC active serial device node, e.g. /dev/ttyAMA4
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_get_fc_port(const mavlink_message_t* msg, char *fc_port)
{
    return _MAV_RETURN_char_array(msg, fc_port, 16,  75);
}

/**
 * @brief Get field siyi_port from cc_telemetry_links message
 *
 * @return  SIYI active serial device node, e.g. /dev/ttyAMA0
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_get_siyi_port(const mavlink_message_t* msg, char *siyi_port)
{
    return _MAV_RETURN_char_array(msg, siyi_port, 16,  91);
}

/**
 * @brief Get field available_ports from cc_telemetry_links message
 *
 * @return  Comma-separated available plugged serial ports detected by CC, e.g. ttyAMA0,ttyAMA4
 */
static inline uint16_t mavlink_msg_cc_telemetry_links_get_available_ports(const mavlink_message_t* msg, char *available_ports)
{
    return _MAV_RETURN_char_array(msg, available_ports, 48,  107);
}

/**
 * @brief Decode a cc_telemetry_links message into a struct
 *
 * @param msg The message to decode
 * @param cc_telemetry_links C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_telemetry_links_decode(const mavlink_message_t* msg, mavlink_cc_telemetry_links_t* cc_telemetry_links)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_telemetry_links->fc_tx_rate = mavlink_msg_cc_telemetry_links_get_fc_tx_rate(msg);
    cc_telemetry_links->fc_rx_rate = mavlink_msg_cc_telemetry_links_get_fc_rx_rate(msg);
    cc_telemetry_links->fc_tx_rate_max = mavlink_msg_cc_telemetry_links_get_fc_tx_rate_max(msg);
    cc_telemetry_links->fc_tx_rate_multi = mavlink_msg_cc_telemetry_links_get_fc_tx_rate_multi(msg);
    cc_telemetry_links->fc_rx_loss = mavlink_msg_cc_telemetry_links_get_fc_rx_loss(msg);
    cc_telemetry_links->fc_tx_err = mavlink_msg_cc_telemetry_links_get_fc_tx_err(msg);
    cc_telemetry_links->fc_bytes_rx = mavlink_msg_cc_telemetry_links_get_fc_bytes_rx(msg);
    cc_telemetry_links->fc_bytes_tx = mavlink_msg_cc_telemetry_links_get_fc_bytes_tx(msg);
    cc_telemetry_links->fc_baudrate = mavlink_msg_cc_telemetry_links_get_fc_baudrate(msg);
    cc_telemetry_links->siyi_tx_rate = mavlink_msg_cc_telemetry_links_get_siyi_tx_rate(msg);
    cc_telemetry_links->siyi_rx_rate = mavlink_msg_cc_telemetry_links_get_siyi_rx_rate(msg);
    cc_telemetry_links->siyi_tx_rate_max = mavlink_msg_cc_telemetry_links_get_siyi_tx_rate_max(msg);
    cc_telemetry_links->siyi_tx_rate_multi = mavlink_msg_cc_telemetry_links_get_siyi_tx_rate_multi(msg);
    cc_telemetry_links->siyi_rx_loss = mavlink_msg_cc_telemetry_links_get_siyi_rx_loss(msg);
    cc_telemetry_links->siyi_tx_err = mavlink_msg_cc_telemetry_links_get_siyi_tx_err(msg);
    cc_telemetry_links->siyi_bytes_rx = mavlink_msg_cc_telemetry_links_get_siyi_bytes_rx(msg);
    cc_telemetry_links->siyi_bytes_tx = mavlink_msg_cc_telemetry_links_get_siyi_bytes_tx(msg);
    cc_telemetry_links->siyi_baudrate = mavlink_msg_cc_telemetry_links_get_siyi_baudrate(msg);
    cc_telemetry_links->fc_status = mavlink_msg_cc_telemetry_links_get_fc_status(msg);
    cc_telemetry_links->siyi_status = mavlink_msg_cc_telemetry_links_get_siyi_status(msg);
    cc_telemetry_links->transport_type = mavlink_msg_cc_telemetry_links_get_transport_type(msg);
    mavlink_msg_cc_telemetry_links_get_fc_port(msg, cc_telemetry_links->fc_port);
    mavlink_msg_cc_telemetry_links_get_siyi_port(msg, cc_telemetry_links->siyi_port);
    mavlink_msg_cc_telemetry_links_get_available_ports(msg, cc_telemetry_links->available_ports);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN? msg->len : MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN;
        memset(cc_telemetry_links, 0, MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_LEN);
    memcpy(cc_telemetry_links, _MAV_PAYLOAD(msg), len);
#endif
}
