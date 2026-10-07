#pragma once
// MESSAGE CC_CONFIG_GET PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_GET 42102


typedef struct __mavlink_cc_config_get_t {
 uint32_t request_id; /*<  Client request ID*/
 char key[64]; /*<  Namespaced configuration key path*/
} mavlink_cc_config_get_t;

#define MAVLINK_MSG_ID_CC_CONFIG_GET_LEN 68
#define MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN 68
#define MAVLINK_MSG_ID_42102_LEN 68
#define MAVLINK_MSG_ID_42102_MIN_LEN 68

#define MAVLINK_MSG_ID_CC_CONFIG_GET_CRC 231
#define MAVLINK_MSG_ID_42102_CRC 231

#define MAVLINK_MSG_CC_CONFIG_GET_FIELD_KEY_LEN 64

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_GET { \
    42102, \
    "CC_CONFIG_GET", \
    2, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_get_t, request_id) }, \
         { "key", NULL, MAVLINK_TYPE_CHAR, 64, 4, offsetof(mavlink_cc_config_get_t, key) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_GET { \
    "CC_CONFIG_GET", \
    2, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_get_t, request_id) }, \
         { "key", NULL, MAVLINK_TYPE_CHAR, 64, 4, offsetof(mavlink_cc_config_get_t, key) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_get message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param key  Namespaced configuration key path
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_get_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t request_id, const char *key)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_GET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#else
    mavlink_cc_config_get_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_GET;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
}

/**
 * @brief Pack a cc_config_get message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param key  Namespaced configuration key path
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_get_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t request_id, const char *key)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_GET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#else
    mavlink_cc_config_get_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_GET;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#endif
}

/**
 * @brief Pack a cc_config_get message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param request_id  Client request ID
 * @param key  Namespaced configuration key path
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_get_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t request_id,const char *key)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_GET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#else
    mavlink_cc_config_get_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_GET;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
}

/**
 * @brief Encode a cc_config_get struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_get C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_get_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_get_t* cc_config_get)
{
    return mavlink_msg_cc_config_get_pack(system_id, component_id, msg, cc_config_get->request_id, cc_config_get->key);
}

/**
 * @brief Encode a cc_config_get struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_get C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_get_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_get_t* cc_config_get)
{
    return mavlink_msg_cc_config_get_pack_chan(system_id, component_id, chan, msg, cc_config_get->request_id, cc_config_get->key);
}

/**
 * @brief Encode a cc_config_get struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_get C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_get_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_get_t* cc_config_get)
{
    return mavlink_msg_cc_config_get_pack_status(system_id, component_id, _status, msg,  cc_config_get->request_id, cc_config_get->key);
}

/**
 * @brief Send a cc_config_get message
 * @param chan MAVLink channel to send the message
 *
 * @param request_id  Client request ID
 * @param key  Namespaced configuration key path
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_get_send(mavlink_channel_t chan, uint32_t request_id, const char *key)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_GET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_GET, buf, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
#else
    mavlink_cc_config_get_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_GET, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
#endif
}

/**
 * @brief Send a cc_config_get message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_get_send_struct(mavlink_channel_t chan, const mavlink_cc_config_get_t* cc_config_get)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_get_send(chan, cc_config_get->request_id, cc_config_get->key);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_GET, (const char *)cc_config_get, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_GET_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_get_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t request_id, const char *key)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_GET, buf, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
#else
    mavlink_cc_config_get_t *packet = (mavlink_cc_config_get_t *)msgbuf;
    packet->request_id = request_id;
    mav_array_memcpy(packet->key, key, sizeof(char)*64);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_GET, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN, MAVLINK_MSG_ID_CC_CONFIG_GET_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_GET UNPACKING


/**
 * @brief Get field request_id from cc_config_get message
 *
 * @return  Client request ID
 */
static inline uint32_t mavlink_msg_cc_config_get_get_request_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field key from cc_config_get message
 *
 * @return  Namespaced configuration key path
 */
static inline uint16_t mavlink_msg_cc_config_get_get_key(const mavlink_message_t* msg, char *key)
{
    return _MAV_RETURN_char_array(msg, key, 64,  4);
}

/**
 * @brief Decode a cc_config_get message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_get C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_get_decode(const mavlink_message_t* msg, mavlink_cc_config_get_t* cc_config_get)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_get->request_id = mavlink_msg_cc_config_get_get_request_id(msg);
    mavlink_msg_cc_config_get_get_key(msg, cc_config_get->key);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_GET_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_GET_LEN;
        memset(cc_config_get, 0, MAVLINK_MSG_ID_CC_CONFIG_GET_LEN);
    memcpy(cc_config_get, _MAV_PAYLOAD(msg), len);
#endif
}
