#pragma once
// MESSAGE CC_CONFIG_ACK PACKING

#define MAVLINK_MSG_ID_CC_CONFIG_ACK 42106


typedef struct __mavlink_cc_config_ack_t {
 uint32_t request_id; /*<  Matching client request ID*/
 uint32_t transaction_id; /*<  Active transaction ID*/
 uint16_t error_code; /*<  Subsystem error code (0=No error)*/
 uint8_t result; /*<  Transaction result status*/
 uint8_t apply_type; /*<  Required system apply action*/
 char message[50]; /*<  Human-readable descriptive feedback*/
} mavlink_cc_config_ack_t;

#define MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN 62
#define MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN 62
#define MAVLINK_MSG_ID_42106_LEN 62
#define MAVLINK_MSG_ID_42106_MIN_LEN 62

#define MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC 94
#define MAVLINK_MSG_ID_42106_CRC 94

#define MAVLINK_MSG_CC_CONFIG_ACK_FIELD_MESSAGE_LEN 50

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_ACK { \
    42106, \
    "CC_CONFIG_ACK", \
    6, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_ack_t, request_id) }, \
         { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_ack_t, transaction_id) }, \
         { "result", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_cc_config_ack_t, result) }, \
         { "apply_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 11, offsetof(mavlink_cc_config_ack_t, apply_type) }, \
         { "error_code", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_cc_config_ack_t, error_code) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 50, 12, offsetof(mavlink_cc_config_ack_t, message) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_CONFIG_ACK { \
    "CC_CONFIG_ACK", \
    6, \
    {  { "request_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_config_ack_t, request_id) }, \
         { "transaction_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_config_ack_t, transaction_id) }, \
         { "result", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_cc_config_ack_t, result) }, \
         { "apply_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 11, offsetof(mavlink_cc_config_ack_t, apply_type) }, \
         { "error_code", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_cc_config_ack_t, error_code) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 50, 12, offsetof(mavlink_cc_config_ack_t, message) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_config_ack message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Matching client request ID
 * @param transaction_id  Active transaction ID
 * @param result  Transaction result status
 * @param apply_type  Required system apply action
 * @param error_code  Subsystem error code (0=No error)
 * @param message  Human-readable descriptive feedback
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_ack_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t transaction_id, uint8_t result, uint8_t apply_type, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_uint16_t(buf, 8, error_code);
    _mav_put_uint8_t(buf, 10, result);
    _mav_put_uint8_t(buf, 11, apply_type);
    _mav_put_char_array(buf, 12, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#else
    mavlink_cc_config_ack_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.result = result;
    packet.apply_type = apply_type;
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_ACK;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
}

/**
 * @brief Pack a cc_config_ack message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param request_id  Matching client request ID
 * @param transaction_id  Active transaction ID
 * @param result  Transaction result status
 * @param apply_type  Required system apply action
 * @param error_code  Subsystem error code (0=No error)
 * @param message  Human-readable descriptive feedback
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_ack_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t request_id, uint32_t transaction_id, uint8_t result, uint8_t apply_type, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_uint16_t(buf, 8, error_code);
    _mav_put_uint8_t(buf, 10, result);
    _mav_put_uint8_t(buf, 11, apply_type);
    _mav_put_char_array(buf, 12, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#else
    mavlink_cc_config_ack_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.result = result;
    packet.apply_type = apply_type;
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_ACK;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#endif
}

/**
 * @brief Pack a cc_config_ack message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param request_id  Matching client request ID
 * @param transaction_id  Active transaction ID
 * @param result  Transaction result status
 * @param apply_type  Required system apply action
 * @param error_code  Subsystem error code (0=No error)
 * @param message  Human-readable descriptive feedback
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_config_ack_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t request_id,uint32_t transaction_id,uint8_t result,uint8_t apply_type,uint16_t error_code,const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_uint16_t(buf, 8, error_code);
    _mav_put_uint8_t(buf, 10, result);
    _mav_put_uint8_t(buf, 11, apply_type);
    _mav_put_char_array(buf, 12, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#else
    mavlink_cc_config_ack_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.result = result;
    packet.apply_type = apply_type;
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_CONFIG_ACK;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
}

/**
 * @brief Encode a cc_config_ack struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_ack C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_ack_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_config_ack_t* cc_config_ack)
{
    return mavlink_msg_cc_config_ack_pack(system_id, component_id, msg, cc_config_ack->request_id, cc_config_ack->transaction_id, cc_config_ack->result, cc_config_ack->apply_type, cc_config_ack->error_code, cc_config_ack->message);
}

/**
 * @brief Encode a cc_config_ack struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_ack C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_ack_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_config_ack_t* cc_config_ack)
{
    return mavlink_msg_cc_config_ack_pack_chan(system_id, component_id, chan, msg, cc_config_ack->request_id, cc_config_ack->transaction_id, cc_config_ack->result, cc_config_ack->apply_type, cc_config_ack->error_code, cc_config_ack->message);
}

/**
 * @brief Encode a cc_config_ack struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_config_ack C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_config_ack_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_config_ack_t* cc_config_ack)
{
    return mavlink_msg_cc_config_ack_pack_status(system_id, component_id, _status, msg,  cc_config_ack->request_id, cc_config_ack->transaction_id, cc_config_ack->result, cc_config_ack->apply_type, cc_config_ack->error_code, cc_config_ack->message);
}

/**
 * @brief Send a cc_config_ack message
 * @param chan MAVLink channel to send the message
 *
 * @param request_id  Matching client request ID
 * @param transaction_id  Active transaction ID
 * @param result  Transaction result status
 * @param apply_type  Required system apply action
 * @param error_code  Subsystem error code (0=No error)
 * @param message  Human-readable descriptive feedback
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_config_ack_send(mavlink_channel_t chan, uint32_t request_id, uint32_t transaction_id, uint8_t result, uint8_t apply_type, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN];
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_uint16_t(buf, 8, error_code);
    _mav_put_uint8_t(buf, 10, result);
    _mav_put_uint8_t(buf, 11, apply_type);
    _mav_put_char_array(buf, 12, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_ACK, buf, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
#else
    mavlink_cc_config_ack_t packet;
    packet.request_id = request_id;
    packet.transaction_id = transaction_id;
    packet.error_code = error_code;
    packet.result = result;
    packet.apply_type = apply_type;
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_ACK, (const char *)&packet, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
#endif
}

/**
 * @brief Send a cc_config_ack message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_config_ack_send_struct(mavlink_channel_t chan, const mavlink_cc_config_ack_t* cc_config_ack)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_config_ack_send(chan, cc_config_ack->request_id, cc_config_ack->transaction_id, cc_config_ack->result, cc_config_ack->apply_type, cc_config_ack->error_code, cc_config_ack->message);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_ACK, (const char *)cc_config_ack, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_config_ack_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t request_id, uint32_t transaction_id, uint8_t result, uint8_t apply_type, uint16_t error_code, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, request_id);
    _mav_put_uint32_t(buf, 4, transaction_id);
    _mav_put_uint16_t(buf, 8, error_code);
    _mav_put_uint8_t(buf, 10, result);
    _mav_put_uint8_t(buf, 11, apply_type);
    _mav_put_char_array(buf, 12, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_ACK, buf, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
#else
    mavlink_cc_config_ack_t *packet = (mavlink_cc_config_ack_t *)msgbuf;
    packet->request_id = request_id;
    packet->transaction_id = transaction_id;
    packet->error_code = error_code;
    packet->result = result;
    packet->apply_type = apply_type;
    mav_array_memcpy(packet->message, message, sizeof(char)*50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_CONFIG_ACK, (const char *)packet, MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN, MAVLINK_MSG_ID_CC_CONFIG_ACK_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_CONFIG_ACK UNPACKING


/**
 * @brief Get field request_id from cc_config_ack message
 *
 * @return  Matching client request ID
 */
static inline uint32_t mavlink_msg_cc_config_ack_get_request_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field transaction_id from cc_config_ack message
 *
 * @return  Active transaction ID
 */
static inline uint32_t mavlink_msg_cc_config_ack_get_transaction_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Get field result from cc_config_ack message
 *
 * @return  Transaction result status
 */
static inline uint8_t mavlink_msg_cc_config_ack_get_result(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  10);
}

/**
 * @brief Get field apply_type from cc_config_ack message
 *
 * @return  Required system apply action
 */
static inline uint8_t mavlink_msg_cc_config_ack_get_apply_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  11);
}

/**
 * @brief Get field error_code from cc_config_ack message
 *
 * @return  Subsystem error code (0=No error)
 */
static inline uint16_t mavlink_msg_cc_config_ack_get_error_code(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  8);
}

/**
 * @brief Get field message from cc_config_ack message
 *
 * @return  Human-readable descriptive feedback
 */
static inline uint16_t mavlink_msg_cc_config_ack_get_message(const mavlink_message_t* msg, char *message)
{
    return _MAV_RETURN_char_array(msg, message, 50,  12);
}

/**
 * @brief Decode a cc_config_ack message into a struct
 *
 * @param msg The message to decode
 * @param cc_config_ack C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_config_ack_decode(const mavlink_message_t* msg, mavlink_cc_config_ack_t* cc_config_ack)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_config_ack->request_id = mavlink_msg_cc_config_ack_get_request_id(msg);
    cc_config_ack->transaction_id = mavlink_msg_cc_config_ack_get_transaction_id(msg);
    cc_config_ack->error_code = mavlink_msg_cc_config_ack_get_error_code(msg);
    cc_config_ack->result = mavlink_msg_cc_config_ack_get_result(msg);
    cc_config_ack->apply_type = mavlink_msg_cc_config_ack_get_apply_type(msg);
    mavlink_msg_cc_config_ack_get_message(msg, cc_config_ack->message);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN? msg->len : MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN;
        memset(cc_config_ack, 0, MAVLINK_MSG_ID_CC_CONFIG_ACK_LEN);
    memcpy(cc_config_ack, _MAV_PAYLOAD(msg), len);
#endif
}
