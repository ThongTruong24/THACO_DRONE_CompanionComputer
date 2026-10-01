#pragma once
// MESSAGE CC_CONFIG_STATUS PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_STATUS 42108


typedef struct __mavlink_cc_config_status_t {
 uint32_t transaction_id; /*<  Active transaction ID*/
 uint16_t error_code; /*<  Subsystem error code if state is error*/
 uint8_t state; /*<  Current lifecycle state*/
 uint8_t progress; /*<  Progress percentage (0 - 100)*/
 char message[50]; /*<  Status description*/
} mavlink_cc_config_status_t;

#define MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN 58
#define MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN 58
#define MAVLINK_MSG_ID_42108_LEN 58
#define MAVLINK_MSG_ID_42108_MIN_LEN 58

#define MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC 156
#define MAVLINK_MSG_ID_42108_CRC 156

#define MAVLINK_MSG_CC_CONFIG_STATUS_FIELD_MESSAGE_LEN 50

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_STATUS { \
    42108, \
    "CC_CONFIG_STATUS", \
    5, \
    {  { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_status_t, transaction_id) }, \
         { "state", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_cc_config_status_t, state) }, \
         { "progress", NULL, MAVLINK_TYPE_UINT8_T, 0, 7, offsetof(mavlink_cc_config_status_t, progress) }, \
         { "error_code", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_cc_config_status_t, error_code) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 50, 8, offsetof(mavlink_cc_config_status_t, message) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_STATUS { \
    "CC_CONFIG_STATUS", \
    5, \
    {  { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_status_t, transaction_id) }, \
         { "state", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_cc_config_status_t, state) }, \
         { "progress", NULL, MAVLINK_TYPE_UINT8_T, 0, 7, offsetof(mavlink_cc_config_status_t, progress) }, \
         { "error_code", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_cc_config_status_t, error_code) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 50, 8, offsetof(mavlink_cc_config_status_t, message) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_status message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param transaction_id  Active transaction ID
 * @param state  Current lifecycle state
 * @param progress  Progress percentage (0 - 100)
 * @param error_code  Subsystem error code if state is error
 * @param message  Status description
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_status_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t transaction_id, uint8_t state, uint8_t progress, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN];
    _mav_put_uint32_t(buf, 0, transaction_id);
    _mav_put_uint16_t(buf, 4, error_code);
    _mav_put_uint8_t(buf, 6, state);
    _mav_put_uint8_t(buf, 7, progress);
    _mav_put_char_array(buf, 8, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#else
    mavlink_cc_config_status_t packet;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.state = state;
    packet.progress = progress;
    mav_array_assign_char(packet.message, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_STATUS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
}

/**
 * @brief Pack a cc_config_status message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param transaction_id  Active transaction ID
 * @param state  Current lifecycle state
 * @param progress  Progress percentage (0 - 100)
 * @param error_code  Subsystem error code if state is error
 * @param message  Status description
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_status_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t transaction_id, uint8_t state, uint8_t progress, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN];
    _mav_put_uint32_t(buf, 0, transaction_id);
    _mav_put_uint16_t(buf, 4, error_code);
    _mav_put_uint8_t(buf, 6, state);
    _mav_put_uint8_t(buf, 7, progress);
    _mav_put_char_array(buf, 8, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#else
    mavlink_cc_config_status_t packet;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.state = state;
    packet.progress = progress;
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_STATUS;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#endif
}

/**
 * @brief Pack a cc_config_status message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param transaction_id  Active transaction ID
 * @param state  Current lifecycle state
 * @param progress  Progress percentage (0 - 100)
 * @param error_code  Subsystem error code if state is error
 * @param message  Status description
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_status_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t transaction_id,uint8_t state,uint8_t progress,uint16_t error_code,const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN];
    _mav_put_uint32_t(buf, 0, transaction_id);
    _mav_put_uint16_t(buf, 4, error_code);
    _mav_put_uint8_t(buf, 6, state);
    _mav_put_uint8_t(buf, 7, progress);
    _mav_put_char_array(buf, 8, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#else
    mavlink_cc_config_status_t packet;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.state = state;
    packet.progress = progress;
    mav_array_assign_char(packet.message, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_STATUS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
}

/**
 * @brief Encode a cc_config_status struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_status_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_status_t* cc_config_status)
{
    return mavlink_msg_cc_config_status_pack(system_id, component_id, msg, cc_config_status->transaction_id, cc_config_status->state, cc_config_status->progress, cc_config_status->error_code, cc_config_status->message);
}

/**
 * @brief Encode a cc_config_status struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_status_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_status_t* cc_config_status)
{
    return mavlink_msg_cc_config_status_pack_chan(system_id, component_id, chan, msg, cc_config_status->transaction_id, cc_config_status->state, cc_config_status->progress, cc_config_status->error_code, cc_config_status->message);
}

/**
 * @brief Encode a cc_config_status struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_status_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_status_t* cc_config_status)
{
    return mavlink_msg_cc_config_status_pack_status(system_id, component_id, _status, msg,  cc_config_status->transaction_id, cc_config_status->state, cc_config_status->progress, cc_config_status->error_code, cc_config_status->message);
}

/**
 * @brief Send a cc_config_status message
 * @param chan MAVLink channel to send the message
 *
 * @param transaction_id  Active transaction ID
 * @param state  Current lifecycle state
 * @param progress  Progress percentage (0 - 100)
 * @param error_code  Subsystem error code if state is error
 * @param message  Status description
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_status_send(mavlink_channel_t chan, uint32_t transaction_id, uint8_t state, uint8_t progress, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN];
    _mav_put_uint32_t(buf, 0, transaction_id);
    _mav_put_uint16_t(buf, 4, error_code);
    _mav_put_uint8_t(buf, 6, state);
    _mav_put_uint8_t(buf, 7, progress);
    _mav_put_char_array(buf, 8, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_STATUS, buf, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
#else
    mavlink_cc_config_status_t packet;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.state = state;
    packet.progress = progress;
    mav_array_assign_char(packet.message, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_STATUS, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
#endif
}

/**
 * @brief Send a cc_config_status message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_status_send_struct(mavlink_channel_t chan, const mavlink_cc_config_status_t* cc_config_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_status_send(chan, cc_config_status->transaction_id, cc_config_status->state, cc_config_status->progress, cc_config_status->error_code, cc_config_status->message);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_STATUS, (const char *)cc_config_status, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_status_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t transaction_id, uint8_t state, uint8_t progress, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, transaction_id);
    _mav_put_uint16_t(buf, 4, error_code);
    _mav_put_uint8_t(buf, 6, state);
    _mav_put_uint8_t(buf, 7, progress);
    _mav_put_char_array(buf, 8, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_STATUS, buf, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
#else
    mavlink_cc_config_status_t *packet = (mavlink_cc_config_status_t *)msgbuf;
    packet->transaction_id = transaction_id;
    packet->error_code = error_code;
    packet->state = state;
    packet->progress = progress;
    mav_array_assign_char(packet->message, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_STATUS, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN, MAVLINK_MSG_ID_CC_CONFIG_STATUS_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_STATUS UNPACKING


/**
 * @brief Get field transaction_id from cc_config_status message
 *
 * @return  Active transaction ID
 */
static inline uint32_t mavlink_msg_cc_config_status_get_transaction_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field state from cc_config_status message
 *
 * @return  Current lifecycle state
 */
static inline uint8_t mavlink_msg_cc_config_status_get_state(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  6);
}

/**
 * @brief Get field progress from cc_config_status message
 *
 * @return  Progress percentage (0 - 100)
 */
static inline uint8_t mavlink_msg_cc_config_status_get_progress(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  7);
}

/**
 * @brief Get field error_code from cc_config_status message
 *
 * @return  Subsystem error code if state is error
 */
static inline uint16_t mavlink_msg_cc_config_status_get_error_code(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  4);
}

/**
 * @brief Get field message from cc_config_status message
 *
 * @return  Status description
 */
static inline uint16_t mavlink_msg_cc_config_status_get_message(const mavlink_message_t* msg, char *message)
{
    return _MAV_RETURN_char_array(msg, message, 50,  8);
}

/**
 * @brief Decode a cc_config_status message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_status C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_status_decode(const mavlink_message_t* msg, mavlink_cc_config_status_t* cc_config_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_status->transaction_id = mavlink_msg_cc_config_status_get_transaction_id(msg);
    cc_config_status->error_code = mavlink_msg_cc_config_status_get_error_code(msg);
    cc_config_status->state = mavlink_msg_cc_config_status_get_state(msg);
    cc_config_status->progress = mavlink_msg_cc_config_status_get_progress(msg);
    mavlink_msg_cc_config_status_get_message(msg, cc_config_status->message);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN;
        memset(cc_config_status, 0, MAVLINK_MSG_ID_CC_CONFIG_STATUS_LEN);
    memcpy(cc_config_status, _MAV_PAYLOAD(msg), len);
#endif
}
