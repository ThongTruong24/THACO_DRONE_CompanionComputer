#pragma once
// MESSAGE CC_SERIAL_LINK PACKING

#define MAVLINK_MSG_ID_CC_SERIAL_LINK 42015


typedef struct __mavlink_cc_serial_link_t {
 float rx_rate; /*< [B/s] MAVLink bytes/s received on this link (router endpoint stats)*/
 float tx_rate; /*< [B/s] MAVLink bytes/s transmitted on this link*/
 float rx_loss; /*< [%] Sequence loss percentage*/
 uint32_t rx_bytes; /*< [bytes] Total bytes received*/
 uint32_t tx_bytes; /*< [bytes] Total bytes transmitted*/
 uint32_t rx_errors; /*<  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise)*/
 uint32_t baudrate; /*< [bps] Configured baud rate*/
 uint8_t link_index; /*<  Index of this link in the active link list (0..link_count-1)*/
 uint8_t link_count; /*<  Number of configured links*/
 uint8_t status; /*<  Link status*/
 char name[16]; /*<  User-defined link name, e.g. FC*/
 char port[16]; /*<  Serial device node, e.g. /dev/ttyAMA4*/
} mavlink_cc_serial_link_t;

#define MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN 63
#define MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN 63
#define MAVLINK_MSG_ID_42015_LEN 63
#define MAVLINK_MSG_ID_42015_MIN_LEN 63

#define MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC 19
#define MAVLINK_MSG_ID_42015_CRC 19

#define MAVLINK_MSG_CC_SERIAL_LINK_FIELD_NAME_LEN 16
#define MAVLINK_MSG_CC_SERIAL_LINK_FIELD_PORT_LEN 16

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_SERIAL_LINK { \
    42015, \
    "CC_SERIAL_LINK", \
    12, \
    {  { "rx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_cc_serial_link_t, rx_rate) }, \
         { "tx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_cc_serial_link_t, tx_rate) }, \
         { "rx_loss", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_cc_serial_link_t, rx_loss) }, \
         { "rx_bytes", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_cc_serial_link_t, rx_bytes) }, \
         { "tx_bytes", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_cc_serial_link_t, tx_bytes) }, \
         { "rx_errors", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_cc_serial_link_t, rx_errors) }, \
         { "baudrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 24, offsetof(mavlink_cc_serial_link_t, baudrate) }, \
         { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_cc_serial_link_t, link_index) }, \
         { "link_count", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_cc_serial_link_t, link_count) }, \
         { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_cc_serial_link_t, status) }, \
         { "name", NULL, MAVLINK_TYPE_CHAR, 16, 31, offsetof(mavlink_cc_serial_link_t, name) }, \
         { "port", NULL, MAVLINK_TYPE_CHAR, 16, 47, offsetof(mavlink_cc_serial_link_t, port) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_SERIAL_LINK { \
    "CC_SERIAL_LINK", \
    12, \
    {  { "rx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_cc_serial_link_t, rx_rate) }, \
         { "tx_rate", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_cc_serial_link_t, tx_rate) }, \
         { "rx_loss", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_cc_serial_link_t, rx_loss) }, \
         { "rx_bytes", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_cc_serial_link_t, rx_bytes) }, \
         { "tx_bytes", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_cc_serial_link_t, tx_bytes) }, \
         { "rx_errors", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_cc_serial_link_t, rx_errors) }, \
         { "baudrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 24, offsetof(mavlink_cc_serial_link_t, baudrate) }, \
         { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_cc_serial_link_t, link_index) }, \
         { "link_count", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_cc_serial_link_t, link_count) }, \
         { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_cc_serial_link_t, status) }, \
         { "name", NULL, MAVLINK_TYPE_CHAR, 16, 31, offsetof(mavlink_cc_serial_link_t, name) }, \
         { "port", NULL, MAVLINK_TYPE_CHAR, 16, 47, offsetof(mavlink_cc_serial_link_t, port) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_serial_link message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param rx_rate [B/s] MAVLink bytes/s received on this link (router endpoint stats)
 * @param tx_rate [B/s] MAVLink bytes/s transmitted on this link
 * @param rx_loss [%] Sequence loss percentage
 * @param rx_bytes [bytes] Total bytes received
 * @param tx_bytes [bytes] Total bytes transmitted
 * @param rx_errors  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise)
 * @param baudrate [bps] Configured baud rate
 * @param link_index  Index of this link in the active link list (0..link_count-1)
 * @param link_count  Number of configured links
 * @param status  Link status
 * @param name  User-defined link name, e.g. FC
 * @param port  Serial device node, e.g. /dev/ttyAMA4
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_serial_link_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               float rx_rate, float tx_rate, float rx_loss, uint32_t rx_bytes, uint32_t tx_bytes, uint32_t rx_errors, uint32_t baudrate, uint8_t link_index, uint8_t link_count, uint8_t status, const char *name, const char *port)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN];
    _mav_put_float(buf, 0, rx_rate);
    _mav_put_float(buf, 4, tx_rate);
    _mav_put_float(buf, 8, rx_loss);
    _mav_put_uint32_t(buf, 12, rx_bytes);
    _mav_put_uint32_t(buf, 16, tx_bytes);
    _mav_put_uint32_t(buf, 20, rx_errors);
    _mav_put_uint32_t(buf, 24, baudrate);
    _mav_put_uint8_t(buf, 28, link_index);
    _mav_put_uint8_t(buf, 29, link_count);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_char_array(buf, 31, name, 16);
    _mav_put_char_array(buf, 47, port, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#else
    mavlink_cc_serial_link_t packet;
    packet.rx_rate = rx_rate;
    packet.tx_rate = tx_rate;
    packet.rx_loss = rx_loss;
    packet.rx_bytes = rx_bytes;
    packet.tx_bytes = tx_bytes;
    packet.rx_errors = rx_errors;
    packet.baudrate = baudrate;
    packet.link_index = link_index;
    packet.link_count = link_count;
    packet.status = status;
    mav_array_memcpy(packet.name, name, sizeof(char)*16);
    mav_array_memcpy(packet.port, port, sizeof(char)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_SERIAL_LINK;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
}

/**
 * @brief Pack a cc_serial_link message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param rx_rate [B/s] MAVLink bytes/s received on this link (router endpoint stats)
 * @param tx_rate [B/s] MAVLink bytes/s transmitted on this link
 * @param rx_loss [%] Sequence loss percentage
 * @param rx_bytes [bytes] Total bytes received
 * @param tx_bytes [bytes] Total bytes transmitted
 * @param rx_errors  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise)
 * @param baudrate [bps] Configured baud rate
 * @param link_index  Index of this link in the active link list (0..link_count-1)
 * @param link_count  Number of configured links
 * @param status  Link status
 * @param name  User-defined link name, e.g. FC
 * @param port  Serial device node, e.g. /dev/ttyAMA4
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_serial_link_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               float rx_rate, float tx_rate, float rx_loss, uint32_t rx_bytes, uint32_t tx_bytes, uint32_t rx_errors, uint32_t baudrate, uint8_t link_index, uint8_t link_count, uint8_t status, const char *name, const char *port)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN];
    _mav_put_float(buf, 0, rx_rate);
    _mav_put_float(buf, 4, tx_rate);
    _mav_put_float(buf, 8, rx_loss);
    _mav_put_uint32_t(buf, 12, rx_bytes);
    _mav_put_uint32_t(buf, 16, tx_bytes);
    _mav_put_uint32_t(buf, 20, rx_errors);
    _mav_put_uint32_t(buf, 24, baudrate);
    _mav_put_uint8_t(buf, 28, link_index);
    _mav_put_uint8_t(buf, 29, link_count);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_char_array(buf, 31, name, 16);
    _mav_put_char_array(buf, 47, port, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#else
    mavlink_cc_serial_link_t packet;
    packet.rx_rate = rx_rate;
    packet.tx_rate = tx_rate;
    packet.rx_loss = rx_loss;
    packet.rx_bytes = rx_bytes;
    packet.tx_bytes = tx_bytes;
    packet.rx_errors = rx_errors;
    packet.baudrate = baudrate;
    packet.link_index = link_index;
    packet.link_count = link_count;
    packet.status = status;
    mav_array_memcpy(packet.name, name, sizeof(char)*16);
    mav_array_memcpy(packet.port, port, sizeof(char)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_SERIAL_LINK;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#endif
}

/**
 * @brief Pack a cc_serial_link message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param rx_rate [B/s] MAVLink bytes/s received on this link (router endpoint stats)
 * @param tx_rate [B/s] MAVLink bytes/s transmitted on this link
 * @param rx_loss [%] Sequence loss percentage
 * @param rx_bytes [bytes] Total bytes received
 * @param tx_bytes [bytes] Total bytes transmitted
 * @param rx_errors  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise)
 * @param baudrate [bps] Configured baud rate
 * @param link_index  Index of this link in the active link list (0..link_count-1)
 * @param link_count  Number of configured links
 * @param status  Link status
 * @param name  User-defined link name, e.g. FC
 * @param port  Serial device node, e.g. /dev/ttyAMA4
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_serial_link_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   float rx_rate,float tx_rate,float rx_loss,uint32_t rx_bytes,uint32_t tx_bytes,uint32_t rx_errors,uint32_t baudrate,uint8_t link_index,uint8_t link_count,uint8_t status,const char *name,const char *port)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN];
    _mav_put_float(buf, 0, rx_rate);
    _mav_put_float(buf, 4, tx_rate);
    _mav_put_float(buf, 8, rx_loss);
    _mav_put_uint32_t(buf, 12, rx_bytes);
    _mav_put_uint32_t(buf, 16, tx_bytes);
    _mav_put_uint32_t(buf, 20, rx_errors);
    _mav_put_uint32_t(buf, 24, baudrate);
    _mav_put_uint8_t(buf, 28, link_index);
    _mav_put_uint8_t(buf, 29, link_count);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_char_array(buf, 31, name, 16);
    _mav_put_char_array(buf, 47, port, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#else
    mavlink_cc_serial_link_t packet;
    packet.rx_rate = rx_rate;
    packet.tx_rate = tx_rate;
    packet.rx_loss = rx_loss;
    packet.rx_bytes = rx_bytes;
    packet.tx_bytes = tx_bytes;
    packet.rx_errors = rx_errors;
    packet.baudrate = baudrate;
    packet.link_index = link_index;
    packet.link_count = link_count;
    packet.status = status;
    mav_array_memcpy(packet.name, name, sizeof(char)*16);
    mav_array_memcpy(packet.port, port, sizeof(char)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_SERIAL_LINK;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
}

/**
 * @brief Encode a cc_serial_link struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_serial_link C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_serial_link_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_serial_link_t* cc_serial_link)
{
    return mavlink_msg_cc_serial_link_pack(system_id, component_id, msg, cc_serial_link->rx_rate, cc_serial_link->tx_rate, cc_serial_link->rx_loss, cc_serial_link->rx_bytes, cc_serial_link->tx_bytes, cc_serial_link->rx_errors, cc_serial_link->baudrate, cc_serial_link->link_index, cc_serial_link->link_count, cc_serial_link->status, cc_serial_link->name, cc_serial_link->port);
}

/**
 * @brief Encode a cc_serial_link struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_serial_link C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_serial_link_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_serial_link_t* cc_serial_link)
{
    return mavlink_msg_cc_serial_link_pack_chan(system_id, component_id, chan, msg, cc_serial_link->rx_rate, cc_serial_link->tx_rate, cc_serial_link->rx_loss, cc_serial_link->rx_bytes, cc_serial_link->tx_bytes, cc_serial_link->rx_errors, cc_serial_link->baudrate, cc_serial_link->link_index, cc_serial_link->link_count, cc_serial_link->status, cc_serial_link->name, cc_serial_link->port);
}

/**
 * @brief Encode a cc_serial_link struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_serial_link C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_serial_link_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_serial_link_t* cc_serial_link)
{
    return mavlink_msg_cc_serial_link_pack_status(system_id, component_id, _status, msg,  cc_serial_link->rx_rate, cc_serial_link->tx_rate, cc_serial_link->rx_loss, cc_serial_link->rx_bytes, cc_serial_link->tx_bytes, cc_serial_link->rx_errors, cc_serial_link->baudrate, cc_serial_link->link_index, cc_serial_link->link_count, cc_serial_link->status, cc_serial_link->name, cc_serial_link->port);
}

/**
 * @brief Send a cc_serial_link message
 * @param chan MAVLink channel to send the message
 *
 * @param rx_rate [B/s] MAVLink bytes/s received on this link (router endpoint stats)
 * @param tx_rate [B/s] MAVLink bytes/s transmitted on this link
 * @param rx_loss [%] Sequence loss percentage
 * @param rx_bytes [bytes] Total bytes received
 * @param tx_bytes [bytes] Total bytes transmitted
 * @param rx_errors  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise)
 * @param baudrate [bps] Configured baud rate
 * @param link_index  Index of this link in the active link list (0..link_count-1)
 * @param link_count  Number of configured links
 * @param status  Link status
 * @param name  User-defined link name, e.g. FC
 * @param port  Serial device node, e.g. /dev/ttyAMA4
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_serial_link_send(mavlink_channel_t chan, float rx_rate, float tx_rate, float rx_loss, uint32_t rx_bytes, uint32_t tx_bytes, uint32_t rx_errors, uint32_t baudrate, uint8_t link_index, uint8_t link_count, uint8_t status, const char *name, const char *port)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN];
    _mav_put_float(buf, 0, rx_rate);
    _mav_put_float(buf, 4, tx_rate);
    _mav_put_float(buf, 8, rx_loss);
    _mav_put_uint32_t(buf, 12, rx_bytes);
    _mav_put_uint32_t(buf, 16, tx_bytes);
    _mav_put_uint32_t(buf, 20, rx_errors);
    _mav_put_uint32_t(buf, 24, baudrate);
    _mav_put_uint8_t(buf, 28, link_index);
    _mav_put_uint8_t(buf, 29, link_count);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_char_array(buf, 31, name, 16);
    _mav_put_char_array(buf, 47, port, 16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_SERIAL_LINK, buf, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
#else
    mavlink_cc_serial_link_t packet;
    packet.rx_rate = rx_rate;
    packet.tx_rate = tx_rate;
    packet.rx_loss = rx_loss;
    packet.rx_bytes = rx_bytes;
    packet.tx_bytes = tx_bytes;
    packet.rx_errors = rx_errors;
    packet.baudrate = baudrate;
    packet.link_index = link_index;
    packet.link_count = link_count;
    packet.status = status;
    mav_array_memcpy(packet.name, name, sizeof(char)*16);
    mav_array_memcpy(packet.port, port, sizeof(char)*16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_SERIAL_LINK, (const char *)&packet, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
#endif
}

/**
 * @brief Send a cc_serial_link message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_serial_link_send_struct(mavlink_channel_t chan, const mavlink_cc_serial_link_t* cc_serial_link)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_serial_link_send(chan, cc_serial_link->rx_rate, cc_serial_link->tx_rate, cc_serial_link->rx_loss, cc_serial_link->rx_bytes, cc_serial_link->tx_bytes, cc_serial_link->rx_errors, cc_serial_link->baudrate, cc_serial_link->link_index, cc_serial_link->link_count, cc_serial_link->status, cc_serial_link->name, cc_serial_link->port);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_SERIAL_LINK, (const char *)cc_serial_link, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_serial_link_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  float rx_rate, float tx_rate, float rx_loss, uint32_t rx_bytes, uint32_t tx_bytes, uint32_t rx_errors, uint32_t baudrate, uint8_t link_index, uint8_t link_count, uint8_t status, const char *name, const char *port)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, rx_rate);
    _mav_put_float(buf, 4, tx_rate);
    _mav_put_float(buf, 8, rx_loss);
    _mav_put_uint32_t(buf, 12, rx_bytes);
    _mav_put_uint32_t(buf, 16, tx_bytes);
    _mav_put_uint32_t(buf, 20, rx_errors);
    _mav_put_uint32_t(buf, 24, baudrate);
    _mav_put_uint8_t(buf, 28, link_index);
    _mav_put_uint8_t(buf, 29, link_count);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_char_array(buf, 31, name, 16);
    _mav_put_char_array(buf, 47, port, 16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_SERIAL_LINK, buf, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
#else
    mavlink_cc_serial_link_t *packet = (mavlink_cc_serial_link_t *)msgbuf;
    packet->rx_rate = rx_rate;
    packet->tx_rate = tx_rate;
    packet->rx_loss = rx_loss;
    packet->rx_bytes = rx_bytes;
    packet->tx_bytes = tx_bytes;
    packet->rx_errors = rx_errors;
    packet->baudrate = baudrate;
    packet->link_index = link_index;
    packet->link_count = link_count;
    packet->status = status;
    mav_array_memcpy(packet->name, name, sizeof(char)*16);
    mav_array_memcpy(packet->port, port, sizeof(char)*16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_SERIAL_LINK, (const char *)packet, MAVLINK_MSG_ID_CC_SERIAL_LINK_MIN_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN, MAVLINK_MSG_ID_CC_SERIAL_LINK_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_SERIAL_LINK UNPACKING


/**
 * @brief Get field rx_rate from cc_serial_link message
 *
 * @return [B/s] MAVLink bytes/s received on this link (router endpoint stats)
 */
static inline float mavlink_msg_cc_serial_link_get_rx_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field tx_rate from cc_serial_link message
 *
 * @return [B/s] MAVLink bytes/s transmitted on this link
 */
static inline float mavlink_msg_cc_serial_link_get_tx_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field rx_loss from cc_serial_link message
 *
 * @return [%] Sequence loss percentage
 */
static inline float mavlink_msg_cc_serial_link_get_rx_loss(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field rx_bytes from cc_serial_link message
 *
 * @return [bytes] Total bytes received
 */
static inline uint32_t mavlink_msg_cc_serial_link_get_rx_bytes(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  12);
}

/**
 * @brief Get field tx_bytes from cc_serial_link message
 *
 * @return [bytes] Total bytes transmitted
 */
static inline uint32_t mavlink_msg_cc_serial_link_get_tx_bytes(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  16);
}

/**
 * @brief Get field rx_errors from cc_serial_link message
 *
 * @return  Kernel UART framing+overrun+parity+break errors since the previous message (wrong baud or noise)
 */
static inline uint32_t mavlink_msg_cc_serial_link_get_rx_errors(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  20);
}

/**
 * @brief Get field baudrate from cc_serial_link message
 *
 * @return [bps] Configured baud rate
 */
static inline uint32_t mavlink_msg_cc_serial_link_get_baudrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  24);
}

/**
 * @brief Get field link_index from cc_serial_link message
 *
 * @return  Index of this link in the active link list (0..link_count-1)
 */
static inline uint8_t mavlink_msg_cc_serial_link_get_link_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  28);
}

/**
 * @brief Get field link_count from cc_serial_link message
 *
 * @return  Number of configured links
 */
static inline uint8_t mavlink_msg_cc_serial_link_get_link_count(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  29);
}

/**
 * @brief Get field status from cc_serial_link message
 *
 * @return  Link status
 */
static inline uint8_t mavlink_msg_cc_serial_link_get_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  30);
}

/**
 * @brief Get field name from cc_serial_link message
 *
 * @return  User-defined link name, e.g. FC
 */
static inline uint16_t mavlink_msg_cc_serial_link_get_name(const mavlink_message_t* msg, char *name)
{
    return _MAV_RETURN_char_array(msg, name, 16,  31);
}

/**
 * @brief Get field port from cc_serial_link message
 *
 * @return  Serial device node, e.g. /dev/ttyAMA4
 */
static inline uint16_t mavlink_msg_cc_serial_link_get_port(const mavlink_message_t* msg, char *port)
{
    return _MAV_RETURN_char_array(msg, port, 16,  47);
}

/**
 * @brief Decode a cc_serial_link message into a struct
 *
 * @param msg The message to decode
 * @param cc_serial_link C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_serial_link_decode(const mavlink_message_t* msg, mavlink_cc_serial_link_t* cc_serial_link)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_serial_link->rx_rate = mavlink_msg_cc_serial_link_get_rx_rate(msg);
    cc_serial_link->tx_rate = mavlink_msg_cc_serial_link_get_tx_rate(msg);
    cc_serial_link->rx_loss = mavlink_msg_cc_serial_link_get_rx_loss(msg);
    cc_serial_link->rx_bytes = mavlink_msg_cc_serial_link_get_rx_bytes(msg);
    cc_serial_link->tx_bytes = mavlink_msg_cc_serial_link_get_tx_bytes(msg);
    cc_serial_link->rx_errors = mavlink_msg_cc_serial_link_get_rx_errors(msg);
    cc_serial_link->baudrate = mavlink_msg_cc_serial_link_get_baudrate(msg);
    cc_serial_link->link_index = mavlink_msg_cc_serial_link_get_link_index(msg);
    cc_serial_link->link_count = mavlink_msg_cc_serial_link_get_link_count(msg);
    cc_serial_link->status = mavlink_msg_cc_serial_link_get_status(msg);
    mavlink_msg_cc_serial_link_get_name(msg, cc_serial_link->name);
    mavlink_msg_cc_serial_link_get_port(msg, cc_serial_link->port);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN? msg->len : MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN;
        memset(cc_serial_link, 0, MAVLINK_MSG_ID_CC_SERIAL_LINK_LEN);
    memcpy(cc_serial_link, _MAV_PAYLOAD(msg), len);
#endif
}
