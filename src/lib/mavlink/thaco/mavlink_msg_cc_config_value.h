#pragma once
// MESSAGE CC_CONFIG_VALUE PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_VALUE 42107


typedef struct __mavlink_cc_config_value_t {
 uint32_t request_id; /*<  Matching client request ID*/
 char key[64]; /*<  Namespaced configuration key path*/
 char value[128]; /*<  Current value encoded as string*/
} mavlink_cc_config_value_t;

#define MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN 196
#define MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN 196
#define MAVLINK_MSG_ID_42107_LEN 196
#define MAVLINK_MSG_ID_42107_MIN_LEN 196

#define MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC 116
#define MAVLINK_MSG_ID_42107_CRC 116

#define MAVLINK_MSG_CC_CONFIG_VALUE_FIELD_KEY_LEN 64
#define MAVLINK_MSG_CC_CONFIG_VALUE_FIELD_VALUE_LEN 128

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_VALUE { \
    42107, \
    "CC_CONFIG_VALUE", \
    3, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_value_t, request_id) }, \
         { "key", NULL, MAVLINK_TYPE_CHAR, 64, 4, offsetof(mavlink_cc_config_value_t, key) }, \
         { "value", NULL, MAVLINK_TYPE_CHAR, 128, 68, offsetof(mavlink_cc_config_value_t, value) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_VALUE { \
    "CC_CONFIG_VALUE", \
    3, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_value_t, request_id) }, \
         { "key", NULL, MAVLINK_TYPE_CHAR, 64, 4, offsetof(mavlink_cc_config_value_t, key) }, \
         { "value", NULL, MAVLINK_TYPE_CHAR, 128, 68, offsetof(mavlink_cc_config_value_t, value) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_value message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Matching client request ID
 * @param key  Namespaced configuration key path
 * @param value  Current value encoded as string
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_value_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t request_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_put_char_array(buf, 68, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#else
    mavlink_cc_config_value_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
    mav_array_memcpy(packet.value, value, sizeof(char)*128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_VALUE;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
}

/**
 * @brief Pack a cc_config_value message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Matching client request ID
 * @param key  Namespaced configuration key path
 * @param value  Current value encoded as string
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_value_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t request_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_put_char_array(buf, 68, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#else
    mavlink_cc_config_value_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
    mav_array_memcpy(packet.value, value, sizeof(char)*128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_VALUE;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#endif
}

/**
 * @brief Pack a cc_config_value message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param request_id  Matching client request ID
 * @param key  Namespaced configuration key path
 * @param value  Current value encoded as string
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_value_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t request_id,const char *key,const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_put_char_array(buf, 68, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#else
    mavlink_cc_config_value_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
    mav_array_memcpy(packet.value, value, sizeof(char)*128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_VALUE;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
}

/**
 * @brief Encode a cc_config_value struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_value C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_value_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_value_t* cc_config_value)
{
    return mavlink_msg_cc_config_value_pack(system_id, component_id, msg, cc_config_value->request_id, cc_config_value->key, cc_config_value->value);
}

/**
 * @brief Encode a cc_config_value struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_value C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_value_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_value_t* cc_config_value)
{
    return mavlink_msg_cc_config_value_pack_chan(system_id, component_id, chan, msg, cc_config_value->request_id, cc_config_value->key, cc_config_value->value);
}

/**
 * @brief Encode a cc_config_value struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_value C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_value_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_value_t* cc_config_value)
{
    return mavlink_msg_cc_config_value_pack_status(system_id, component_id, _status, msg,  cc_config_value->request_id, cc_config_value->key, cc_config_value->value);
}

/**
 * @brief Send a cc_config_value message
 * @param chan MAVLink channel to send the message
 *
 * @param request_id  Matching client request ID
 * @param key  Namespaced configuration key path
 * @param value  Current value encoded as string
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_value_send(mavlink_channel_t chan, uint32_t request_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_put_char_array(buf, 68, value, 128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_VALUE, buf, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
#else
    mavlink_cc_config_value_t packet;
    packet.request_id = request_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
    mav_array_memcpy(packet.value, value, sizeof(char)*128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_VALUE, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
#endif
}

/**
 * @brief Send a cc_config_value message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_value_send_struct(mavlink_channel_t chan, const mavlink_cc_config_value_t* cc_config_value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_value_send(chan, cc_config_value->request_id, cc_config_value->key, cc_config_value->value);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_VALUE, (const char *)cc_config_value, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_value_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t request_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_char_array(buf, 4, key, 64);
    _mav_put_char_array(buf, 68, value, 128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_VALUE, buf, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
#else
    mavlink_cc_config_value_t *packet = (mavlink_cc_config_value_t *)msgbuf;
    packet->request_id = request_id;
    mav_array_memcpy(packet->key, key, sizeof(char)*64);
    mav_array_memcpy(packet->value, value, sizeof(char)*128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_VALUE, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN, MAVLINK_MSG_ID_CC_CONFIG_VALUE_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_VALUE UNPACKING


/**
 * @brief Get field request_id from cc_config_value message
 *
 * @return  Matching client request ID
 */
static inline uint32_t mavlink_msg_cc_config_value_get_request_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field key from cc_config_value message
 *
 * @return  Namespaced configuration key path
 */
static inline uint16_t mavlink_msg_cc_config_value_get_key(const mavlink_message_t* msg, char *key)
{
    return _MAV_RETURN_char_array(msg, key, 64,  4);
}

/**
 * @brief Get field value from cc_config_value message
 *
 * @return  Current value encoded as string
 */
static inline uint16_t mavlink_msg_cc_config_value_get_value(const mavlink_message_t* msg, char *value)
{
    return _MAV_RETURN_char_array(msg, value, 128,  68);
}

/**
 * @brief Decode a cc_config_value message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_value C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_value_decode(const mavlink_message_t* msg, mavlink_cc_config_value_t* cc_config_value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_value->request_id = mavlink_msg_cc_config_value_get_request_id(msg);
    mavlink_msg_cc_config_value_get_key(msg, cc_config_value->key);
    mavlink_msg_cc_config_value_get_value(msg, cc_config_value->value);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN;
        memset(cc_config_value, 0, MAVLINK_MSG_ID_CC_CONFIG_VALUE_LEN);
    memcpy(cc_config_value, _MAV_PAYLOAD(msg), len);
#endif
}
