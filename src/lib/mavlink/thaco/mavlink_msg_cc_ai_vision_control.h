#pragma once
// MESSAGE CC_AI_VISION_CONTROL PACKING

#define MAVLINK_MSG_ID_CC_AI_VISION_CONTROL 42015


typedef struct __mavlink_cc_ai_vision_control_t {
 uint8_t bounding_box; /*<  Bounding box display control: 0 disabled, 1 enabled*/
 uint8_t tracking; /*<  AI target tracking control: 0 disabled, 1 enabled*/
 uint8_t following; /*<  Target following control: 0 disabled, 1 enabled*/
} mavlink_cc_ai_vision_control_t;

#define MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN 3
#define MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN 3
#define MAVLINK_MSG_ID_42015_LEN 3
#define MAVLINK_MSG_ID_42015_MIN_LEN 3

#define MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC 211
#define MAVLINK_MSG_ID_42015_CRC 211



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_AI_VISION_CONTROL { \
    42015, \
    "CC_AI_VISION_CONTROL", \
    3, \
    {  { "bounding_box", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_cc_ai_vision_control_t, bounding_box) }, \
         { "tracking", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_cc_ai_vision_control_t, tracking) }, \
         { "following", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_cc_ai_vision_control_t, following) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_AI_VISION_CONTROL { \
    "CC_AI_VISION_CONTROL", \
    3, \
    {  { "bounding_box", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_cc_ai_vision_control_t, bounding_box) }, \
         { "tracking", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_cc_ai_vision_control_t, tracking) }, \
         { "following", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_cc_ai_vision_control_t, following) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_ai_vision_control message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param bounding_box  Bounding box display control: 0 disabled, 1 enabled
 * @param tracking  AI target tracking control: 0 disabled, 1 enabled
 * @param following  Target following control: 0 disabled, 1 enabled
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_ai_vision_control_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t bounding_box, uint8_t tracking, uint8_t following)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN];
    _mav_put_uint8_t(buf, 0, bounding_box);
    _mav_put_uint8_t(buf, 1, tracking);
    _mav_put_uint8_t(buf, 2, following);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#else
    mavlink_cc_ai_vision_control_t packet;
    packet.bounding_box = bounding_box;
    packet.tracking = tracking;
    packet.following = following;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_AI_VISION_CONTROL;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
}

/**
 * @brief Pack a cc_ai_vision_control message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param bounding_box  Bounding box display control: 0 disabled, 1 enabled
 * @param tracking  AI target tracking control: 0 disabled, 1 enabled
 * @param following  Target following control: 0 disabled, 1 enabled
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_ai_vision_control_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint8_t bounding_box, uint8_t tracking, uint8_t following)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN];
    _mav_put_uint8_t(buf, 0, bounding_box);
    _mav_put_uint8_t(buf, 1, tracking);
    _mav_put_uint8_t(buf, 2, following);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#else
    mavlink_cc_ai_vision_control_t packet;
    packet.bounding_box = bounding_box;
    packet.tracking = tracking;
    packet.following = following;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_AI_VISION_CONTROL;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#endif
}

/**
 * @brief Pack a cc_ai_vision_control message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param bounding_box  Bounding box display control: 0 disabled, 1 enabled
 * @param tracking  AI target tracking control: 0 disabled, 1 enabled
 * @param following  Target following control: 0 disabled, 1 enabled
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_ai_vision_control_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t bounding_box,uint8_t tracking,uint8_t following)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN];
    _mav_put_uint8_t(buf, 0, bounding_box);
    _mav_put_uint8_t(buf, 1, tracking);
    _mav_put_uint8_t(buf, 2, following);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#else
    mavlink_cc_ai_vision_control_t packet;
    packet.bounding_box = bounding_box;
    packet.tracking = tracking;
    packet.following = following;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_AI_VISION_CONTROL;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
}

/**
 * @brief Encode a cc_ai_vision_control struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_ai_vision_control C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_ai_vision_control_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_ai_vision_control_t* cc_ai_vision_control)
{
    return mavlink_msg_cc_ai_vision_control_pack(system_id, component_id, msg, cc_ai_vision_control->bounding_box, cc_ai_vision_control->tracking, cc_ai_vision_control->following);
}

/**
 * @brief Encode a cc_ai_vision_control struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_ai_vision_control C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_ai_vision_control_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_ai_vision_control_t* cc_ai_vision_control)
{
    return mavlink_msg_cc_ai_vision_control_pack_chan(system_id, component_id, chan, msg, cc_ai_vision_control->bounding_box, cc_ai_vision_control->tracking, cc_ai_vision_control->following);
}

/**
 * @brief Encode a cc_ai_vision_control struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_ai_vision_control C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_ai_vision_control_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_ai_vision_control_t* cc_ai_vision_control)
{
    return mavlink_msg_cc_ai_vision_control_pack_status(system_id, component_id, _status, msg,  cc_ai_vision_control->bounding_box, cc_ai_vision_control->tracking, cc_ai_vision_control->following);
}

/**
 * @brief Send a cc_ai_vision_control message
 * @param chan MAVLink channel to send the message
 *
 * @param bounding_box  Bounding box display control: 0 disabled, 1 enabled
 * @param tracking  AI target tracking control: 0 disabled, 1 enabled
 * @param following  Target following control: 0 disabled, 1 enabled
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_ai_vision_control_send(mavlink_channel_t chan, uint8_t bounding_box, uint8_t tracking, uint8_t following)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN];
    _mav_put_uint8_t(buf, 0, bounding_box);
    _mav_put_uint8_t(buf, 1, tracking);
    _mav_put_uint8_t(buf, 2, following);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL, buf, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
#else
    mavlink_cc_ai_vision_control_t packet;
    packet.bounding_box = bounding_box;
    packet.tracking = tracking;
    packet.following = following;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL, (const char *)&packet, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
#endif
}

/**
 * @brief Send a cc_ai_vision_control message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_ai_vision_control_send_struct(mavlink_channel_t chan, const mavlink_cc_ai_vision_control_t* cc_ai_vision_control)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_ai_vision_control_send(chan, cc_ai_vision_control->bounding_box, cc_ai_vision_control->tracking, cc_ai_vision_control->following);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL, (const char *)cc_ai_vision_control, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_ai_vision_control_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t bounding_box, uint8_t tracking, uint8_t following)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 0, bounding_box);
    _mav_put_uint8_t(buf, 1, tracking);
    _mav_put_uint8_t(buf, 2, following);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL, buf, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
#else
    mavlink_cc_ai_vision_control_t *packet = (mavlink_cc_ai_vision_control_t *)msgbuf;
    packet->bounding_box = bounding_box;
    packet->tracking = tracking;
    packet->following = following;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL, (const char *)packet, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_MIN_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_AI_VISION_CONTROL UNPACKING


/**
 * @brief Get field bounding_box from cc_ai_vision_control message
 *
 * @return  Bounding box display control: 0 disabled, 1 enabled
 */
static inline uint8_t mavlink_msg_cc_ai_vision_control_get_bounding_box(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field tracking from cc_ai_vision_control message
 *
 * @return  AI target tracking control: 0 disabled, 1 enabled
 */
static inline uint8_t mavlink_msg_cc_ai_vision_control_get_tracking(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field following from cc_ai_vision_control message
 *
 * @return  Target following control: 0 disabled, 1 enabled
 */
static inline uint8_t mavlink_msg_cc_ai_vision_control_get_following(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Decode a cc_ai_vision_control message into a struct
 *
 * @param msg The message to decode
 * @param cc_ai_vision_control C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_ai_vision_control_decode(const mavlink_message_t* msg, mavlink_cc_ai_vision_control_t* cc_ai_vision_control)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_ai_vision_control->bounding_box = mavlink_msg_cc_ai_vision_control_get_bounding_box(msg);
    cc_ai_vision_control->tracking = mavlink_msg_cc_ai_vision_control_get_tracking(msg);
    cc_ai_vision_control->following = mavlink_msg_cc_ai_vision_control_get_following(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN? msg->len : MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN;
        memset(cc_ai_vision_control, 0, MAVLINK_MSG_ID_CC_AI_VISION_CONTROL_LEN);
    memcpy(cc_ai_vision_control, _MAV_PAYLOAD(msg), len);
#endif
}
