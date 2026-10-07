#pragma once
// MESSAGE CC_CONFIG_BEGIN PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_BEGIN 42100


typedef struct __mavlink_cc_config_begin_t {
 uint32_t request_id; /*<  Client request ID*/
 uint32_t timeout_s; /*<  Safety confirmation timeout in seconds (default 30)*/
} mavlink_cc_config_begin_t;

#define MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN 8
#define MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN 8
#define MAVLINK_MSG_ID_42100_LEN 8
#define MAVLINK_MSG_ID_42100_MIN_LEN 8

#define MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC 14
#define MAVLINK_MSG_ID_42100_CRC 14



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_BEGIN { \
    42100, \
    "CC_CONFIG_BEGIN", \
    2, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_begin_t, request_id) }, \
         { "timeout_s", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_begin_t, timeout_s) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_BEGIN { \
    "CC_CONFIG_BEGIN", \
    2, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_begin_t, request_id) }, \
         { "timeout_s", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_begin_t, timeout_s) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_begin message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param timeout_s  Safety confirmation timeout in seconds (default 30)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_begin_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t timeout_s)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, timeout_s);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#else
    mavlink_cc_config_begin_t packet;
    packet.request_id = request_id;
    packet.timeout_s = timeout_s;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_BEGIN;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
}

/**
 * @brief Pack a cc_config_begin message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param timeout_s  Safety confirmation timeout in seconds (default 30)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_begin_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t timeout_s)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, timeout_s);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#else
    mavlink_cc_config_begin_t packet;
    packet.request_id = request_id;
    packet.timeout_s = timeout_s;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_BEGIN;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#endif
}

/**
 * @brief Pack a cc_config_begin message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param request_id  Client request ID
 * @param timeout_s  Safety confirmation timeout in seconds (default 30)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_begin_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t request_id,uint32_t timeout_s)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, timeout_s);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#else
    mavlink_cc_config_begin_t packet;
    packet.request_id = request_id;
    packet.timeout_s = timeout_s;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_BEGIN;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
}

/**
 * @brief Encode a cc_config_begin struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_begin C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_begin_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_begin_t* cc_config_begin)
{
    return mavlink_msg_cc_config_begin_pack(system_id, component_id, msg, cc_config_begin->request_id, cc_config_begin->timeout_s);
}

/**
 * @brief Encode a cc_config_begin struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_begin C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_begin_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_begin_t* cc_config_begin)
{
    return mavlink_msg_cc_config_begin_pack_chan(system_id, component_id, chan, msg, cc_config_begin->request_id, cc_config_begin->timeout_s);
}

/**
 * @brief Encode a cc_config_begin struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_begin C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_begin_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_begin_t* cc_config_begin)
{
    return mavlink_msg_cc_config_begin_pack_status(system_id, component_id, _status, msg,  cc_config_begin->request_id, cc_config_begin->timeout_s);
}

/**
 * @brief Send a cc_config_begin message
 * @param chan MAVLink channel to send the message
 *
 * @param request_id  Client request ID
 * @param timeout_s  Safety confirmation timeout in seconds (default 30)
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_begin_send(mavlink_channel_t chan, uint32_t request_id, uint32_t timeout_s)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, timeout_s);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_BEGIN, buf, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
#else
    mavlink_cc_config_begin_t packet;
    packet.request_id = request_id;
    packet.timeout_s = timeout_s;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_BEGIN, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
#endif
}

/**
 * @brief Send a cc_config_begin message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_begin_send_struct(mavlink_channel_t chan, const mavlink_cc_config_begin_t* cc_config_begin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_begin_send(chan, cc_config_begin->request_id, cc_config_begin->timeout_s);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_BEGIN, (const char *)cc_config_begin, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_begin_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t request_id, uint32_t timeout_s)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, timeout_s);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_BEGIN, buf, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
#else
    mavlink_cc_config_begin_t *packet = (mavlink_cc_config_begin_t *)msgbuf;
    packet->request_id = request_id;
    packet->timeout_s = timeout_s;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_BEGIN, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_BEGIN UNPACKING


/**
 * @brief Get field request_id from cc_config_begin message
 *
 * @return  Client request ID
 */
static inline uint32_t mavlink_msg_cc_config_begin_get_request_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field timeout_s from cc_config_begin message
 *
 * @return  Safety confirmation timeout in seconds (default 30)
 */
static inline uint32_t mavlink_msg_cc_config_begin_get_timeout_s(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Decode a cc_config_begin message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_begin C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_begin_decode(const mavlink_message_t* msg, mavlink_cc_config_begin_t* cc_config_begin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_begin->request_id = mavlink_msg_cc_config_begin_get_request_id(msg);
    cc_config_begin->timeout_s = mavlink_msg_cc_config_begin_get_timeout_s(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN;
        memset(cc_config_begin, 0, MAVLINK_MSG_ID_CC_CONFIG_BEGIN_LEN);
    memcpy(cc_config_begin, _MAV_PAYLOAD(msg), len);
#endif
}
