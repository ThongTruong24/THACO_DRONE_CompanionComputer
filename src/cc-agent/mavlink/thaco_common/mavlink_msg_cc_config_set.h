#pragma once
// MESSAGE CC_CONFIG_SET PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_SET 42101


typedef struct __mavlink_cc_config_set_t {
 uint32_t request_id; /*<  Client request ID*/
 uint32_t transaction_id; /*<  Active transaction ID*/
 char key[64]; /*<  Namespaced configuration key path*/
 char value[128]; /*<  New value encoded as string*/
} mavlink_cc_config_set_t;

#define MAVLINK_MSG_ID_CC_CONFIG_SET_LEN 200
#define MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN 200
#define MAVLINK_MSG_ID_42101_LEN 200
#define MAVLINK_MSG_ID_42101_MIN_LEN 200

#define MAVLINK_MSG_ID_CC_CONFIG_SET_CRC 182
#define MAVLINK_MSG_ID_42101_CRC 182

#define MAVLINK_MSG_CC_CONFIG_SET_FIELD_KEY_LEN 64
#define MAVLINK_MSG_CC_CONFIG_SET_FIELD_VALUE_LEN 128

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_SET { \
    42101, \
    "CC_CONFIG_SET", \
    4, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_set_t, request_id) }, \
         { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_set_t, transaction_id) }, \
         { "key", NULL, MAVLINK_TYPE_CHAR, 64, 8, offsetof(mavlink_cc_config_set_t, key) }, \
         { "value", NULL, MAVLINK_TYPE_CHAR, 128, 72, offsetof(mavlink_cc_config_set_t, value) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_SET { \
    "CC_CONFIG_SET", \
    4, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_set_t, request_id) }, \
         { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_set_t, transaction_id) }, \
         { "key", NULL, MAVLINK_TYPE_CHAR, 64, 8, offsetof(mavlink_cc_config_set_t, key) }, \
         { "value", NULL, MAVLINK_TYPE_CHAR, 128, 72, offsetof(mavlink_cc_config_set_t, value) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_set message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param transaction_id  Active transaction ID
 * @param key  Namespaced configuration key path
 * @param value  New value encoded as string
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_set_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t transaction_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_SET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_char_array(buf, 8, key, 64);
    _mav_put_char_array(buf, 72, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#else
    mavlink_cc_config_set_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    mav_array_assign_char(packet.key, key, 64);
    mav_array_assign_char(packet.value, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_SET;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
}

/**
 * @brief Pack a cc_config_set message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Client request ID
 * @param transaction_id  Active transaction ID
 * @param key  Namespaced configuration key path
 * @param value  New value encoded as string
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_set_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t transaction_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_SET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_char_array(buf, 8, key, 64);
    _mav_put_char_array(buf, 72, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#else
    mavlink_cc_config_set_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    mav_array_memcpy(packet.key, key, sizeof(char)*64);
    mav_array_memcpy(packet.value, value, sizeof(char)*128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_SET;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#endif
}

/**
 * @brief Pack a cc_config_set message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param request_id  Client request ID
 * @param transaction_id  Active transaction ID
 * @param key  Namespaced configuration key path
 * @param value  New value encoded as string
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_set_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t request_id,uint32_t transaction_id,const char *key,const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_SET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_char_array(buf, 8, key, 64);
    _mav_put_char_array(buf, 72, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#else
    mavlink_cc_config_set_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    mav_array_assign_char(packet.key, key, 64);
    mav_array_assign_char(packet.value, value, 128);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_SET;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
}

/**
 * @brief Encode a cc_config_set struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_set C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_set_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_set_t* cc_config_set)
{
    return mavlink_msg_cc_config_set_pack(system_id, component_id, msg, cc_config_set->request_id, cc_config_set->transaction_id, cc_config_set->key, cc_config_set->value);
}

/**
 * @brief Encode a cc_config_set struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_set C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_set_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_set_t* cc_config_set)
{
    return mavlink_msg_cc_config_set_pack_chan(system_id, component_id, chan, msg, cc_config_set->request_id, cc_config_set->transaction_id, cc_config_set->key, cc_config_set->value);
}

/**
 * @brief Encode a cc_config_set struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_set C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_set_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_set_t* cc_config_set)
{
    return mavlink_msg_cc_config_set_pack_status(system_id, component_id, _status, msg,  cc_config_set->request_id, cc_config_set->transaction_id, cc_config_set->key, cc_config_set->value);
}

/**
 * @brief Send a cc_config_set message
 * @param chan MAVLink channel to send the message
 *
 * @param request_id  Client request ID
 * @param transaction_id  Active transaction ID
 * @param key  Namespaced configuration key path
 * @param value  New value encoded as string
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_set_send(mavlink_channel_t chan, uint32_t request_id, uint32_t transaction_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_SET_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_char_array(buf, 8, key, 64);
    _mav_put_char_array(buf, 72, value, 128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_SET, buf, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
#else
    mavlink_cc_config_set_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    mav_array_assign_char(packet.key, key, 64);
    mav_array_assign_char(packet.value, value, 128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_SET, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
#endif
}

/**
 * @brief Send a cc_config_set message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_set_send_struct(mavlink_channel_t chan, const mavlink_cc_config_set_t* cc_config_set)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_set_send(chan, cc_config_set->request_id, cc_config_set->transaction_id, cc_config_set->key, cc_config_set->value);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_SET, (const char *)cc_config_set, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_SET_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_set_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t request_id, uint32_t transaction_id, const char *key, const char *value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_char_array(buf, 8, key, 64);
    _mav_put_char_array(buf, 72, value, 128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_SET, buf, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
#else
    mavlink_cc_config_set_t *packet = (mavlink_cc_config_set_t *)msgbuf;
    packet->request_id = request_id;
    packet->transaction_id = transaction_id;
    mav_array_assign_char(packet->key, key, 64);
    mav_array_assign_char(packet->value, value, 128);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_SET, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN, MAVLINK_MSG_ID_CC_CONFIG_SET_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_SET UNPACKING


/**
 * @brief Get field request_id from cc_config_set message
 *
 * @return  Client request ID
 */
static inline uint32_t mavlink_msg_cc_config_set_get_request_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field transaction_id from cc_config_set message
 *
 * @return  Active transaction ID
 */
static inline uint32_t mavlink_msg_cc_config_set_get_transaction_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Get field key from cc_config_set message
 *
 * @return  Namespaced configuration key path
 */
static inline uint16_t mavlink_msg_cc_config_set_get_key(const mavlink_message_t* msg, char *key)
{
    return _MAV_RETURN_char_array(msg, key, 64,  8);
}

/**
 * @brief Get field value from cc_config_set message
 *
 * @return  New value encoded as string
 */
static inline uint16_t mavlink_msg_cc_config_set_get_value(const mavlink_message_t* msg, char *value)
{
    return _MAV_RETURN_char_array(msg, value, 128,  72);
}

/**
 * @brief Decode a cc_config_set message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_set C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_set_decode(const mavlink_message_t* msg, mavlink_cc_config_set_t* cc_config_set)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_set->request_id = mavlink_msg_cc_config_set_get_request_id(msg);
    cc_config_set->transaction_id = mavlink_msg_cc_config_set_get_transaction_id(msg);
    mavlink_msg_cc_config_set_get_key(msg, cc_config_set->key);
    mavlink_msg_cc_config_set_get_value(msg, cc_config_set->value);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_SET_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_SET_LEN;
        memset(cc_config_set, 0, MAVLINK_MSG_ID_CC_CONFIG_SET_LEN);
    memcpy(cc_config_set, _MAV_PAYLOAD(msg), len);
#endif
}
