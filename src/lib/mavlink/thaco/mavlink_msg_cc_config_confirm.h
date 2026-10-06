#pragma once
// MESSAGE CC_CONFIG_CONFIRM PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_CONFIRM 42104


typedef struct __mavlink_cc_config_confirm_t {
 uint32_t request_id; /*<  Client request ID*/
 uint32_t transaction_id; /*<  Transaction ID to confirm*/
} mavlink_cc_config_confirm_t;

#define MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN 8
#define MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN 8
#define MAVLINK_MSG_ID_42104_LEN 8
#define MAVLINK_MSG_ID_42104_MIN_LEN 8

#define MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC 4
#define MAVLINK_MSG_ID_42104_CRC 4



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_CONFIRM { \
    42104, \
    "CC_CONFIG_CONFIRM", \
    2, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_confirm_t, request_id) }, \
         { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_confirm_t, transaction_id) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_CONFIRM { \
    "CC_CONFIG_CONFIRM", \
    2, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_confirm_t, request_id) }, \
         { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_confirm_t, transaction_id) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_confirm message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param transaction_id  Transaction ID to confirm
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_confirm_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t transaction_id)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#else
    mavlink_cc_config_confirm_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_CONFIRM;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
}

/**
 * @brief Pack a cc_config_confirm message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param transaction_id  Transaction ID to confirm
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_confirm_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t transaction_id)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#else
    mavlink_cc_config_confirm_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_CONFIRM;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#endif
}

/**
 * @brief Pack a cc_config_confirm message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param request_id  Client request ID
 * @param transaction_id  Transaction ID to confirm
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_confirm_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t request_id,uint32_t transaction_id)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#else
    mavlink_cc_config_confirm_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_CONFIRM;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
}

/**
 * @brief Encode a cc_config_confirm struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_confirm C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_confirm_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_confirm_t* cc_config_confirm)
{
    return mavlink_msg_cc_config_confirm_pack(system_id, component_id, msg, cc_config_confirm->request_id, cc_config_confirm->transaction_id);
}

/**
 * @brief Encode a cc_config_confirm struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_confirm C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_confirm_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_confirm_t* cc_config_confirm)
{
    return mavlink_msg_cc_config_confirm_pack_chan(system_id, component_id, chan, msg, cc_config_confirm->request_id, cc_config_confirm->transaction_id);
}

/**
 * @brief Encode a cc_config_confirm struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_confirm C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_confirm_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_confirm_t* cc_config_confirm)
{
    return mavlink_msg_cc_config_confirm_pack_status(system_id, component_id, _status, msg,  cc_config_confirm->request_id, cc_config_confirm->transaction_id);
}

/**
 * @brief Send a cc_config_confirm message
 * @param chan MAVLink channel to send the message
 *
 * @param request_id  Client request ID
 * @param transaction_id  Transaction ID to confirm
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_confirm_send(mavlink_channel_t chan, uint32_t request_id, uint32_t transaction_id)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM, buf, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
#else
    mavlink_cc_config_confirm_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
#endif
}

/**
 * @brief Send a cc_config_confirm message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_confirm_send_struct(mavlink_channel_t chan, const mavlink_cc_config_confirm_t* cc_config_confirm)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_confirm_send(chan, cc_config_confirm->request_id, cc_config_confirm->transaction_id);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM, (const char *)cc_config_confirm, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_confirm_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t request_id, uint32_t transaction_id)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM, buf, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
#else
    mavlink_cc_config_confirm_t *packet = (mavlink_cc_config_confirm_t *)msgbuf;
    packet->request_id = request_id;
    packet->transaction_id = transaction_id;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_CONFIRM UNPACKING


/**
 * @brief Get field request_id from cc_config_confirm message
 *
 * @return  Client request ID
 */
static inline uint32_t mavlink_msg_cc_config_confirm_get_request_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field transaction_id from cc_config_confirm message
 *
 * @return  Transaction ID to confirm
 */
static inline uint32_t mavlink_msg_cc_config_confirm_get_transaction_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Decode a cc_config_confirm message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_confirm C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_confirm_decode(const mavlink_message_t* msg, mavlink_cc_config_confirm_t* cc_config_confirm)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_confirm->request_id = mavlink_msg_cc_config_confirm_get_request_id(msg);
    cc_config_confirm->transaction_id = mavlink_msg_cc_config_confirm_get_transaction_id(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN;
        memset(cc_config_confirm, 0, MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_LEN);
    memcpy(cc_config_confirm, _MAV_PAYLOAD(msg), len);
#endif
}
