#pragma once
// MESSAGE THACO_EXTERNAL_XYZ_TRIGGER PACKING

#define MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER 32000


typedef struct __mavlink_thaco_external_xyz_trigger_t {
 uint32_t trigger_id; /*<  Stable per-handoff trigger ID in range 1..16777216*/
 uint32_t time_boot_ms; /*<  Timestamp in milliseconds since system boot*/
} mavlink_thaco_external_xyz_trigger_t;

#define MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN 8
#define MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN 8
#define MAVLINK_MSG_ID_32000_LEN 8
#define MAVLINK_MSG_ID_32000_MIN_LEN 8

#define MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC 239
#define MAVLINK_MSG_ID_32000_CRC 239



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_THACO_EXTERNAL_XYZ_TRIGGER { \
    32000, \
    "THACO_EXTERNAL_XYZ_TRIGGER", \
    2, \
    {  { "trigger_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_thaco_external_xyz_trigger_t, trigger_id) }, \
         { "time_boot_ms", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_thaco_external_xyz_trigger_t, time_boot_ms) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_THACO_EXTERNAL_XYZ_TRIGGER { \
    "THACO_EXTERNAL_XYZ_TRIGGER", \
    2, \
    {  { "trigger_id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_thaco_external_xyz_trigger_t, trigger_id) }, \
         { "time_boot_ms", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_thaco_external_xyz_trigger_t, time_boot_ms) }, \
         } \
}
#endif

/**
 * @brief Pack a thaco_external_xyz_trigger message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param trigger_id  Stable per-handoff trigger ID in range 1..16777216
 * @param time_boot_ms  Timestamp in milliseconds since system boot
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_thaco_external_xyz_trigger_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t trigger_id, uint32_t time_boot_ms)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN];
    _mav_put_uint32_t(buf, 0, trigger_id);
    _mav_put_uint32_t(buf, 4, time_boot_ms);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#else
    mavlink_thaco_external_xyz_trigger_t packet;
    packet.trigger_id = trigger_id;
    packet.time_boot_ms = time_boot_ms;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
}

/**
 * @brief Pack a thaco_external_xyz_trigger message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param trigger_id  Stable per-handoff trigger ID in range 1..16777216
 * @param time_boot_ms  Timestamp in milliseconds since system boot
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_thaco_external_xyz_trigger_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t trigger_id, uint32_t time_boot_ms)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN];
    _mav_put_uint32_t(buf, 0, trigger_id);
    _mav_put_uint32_t(buf, 4, time_boot_ms);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#else
    mavlink_thaco_external_xyz_trigger_t packet;
    packet.trigger_id = trigger_id;
    packet.time_boot_ms = time_boot_ms;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#endif
}

/**
 * @brief Pack a thaco_external_xyz_trigger message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param trigger_id  Stable per-handoff trigger ID in range 1..16777216
 * @param time_boot_ms  Timestamp in milliseconds since system boot
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_thaco_external_xyz_trigger_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t trigger_id,uint32_t time_boot_ms)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN];
    _mav_put_uint32_t(buf, 0, trigger_id);
    _mav_put_uint32_t(buf, 4, time_boot_ms);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#else
    mavlink_thaco_external_xyz_trigger_t packet;
    packet.trigger_id = trigger_id;
    packet.time_boot_ms = time_boot_ms;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
}

/**
 * @brief Encode a thaco_external_xyz_trigger struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param thaco_external_xyz_trigger C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_thaco_external_xyz_trigger_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_thaco_external_xyz_trigger_t* thaco_external_xyz_trigger)
{
    return mavlink_msg_thaco_external_xyz_trigger_pack(system_id, component_id, msg, thaco_external_xyz_trigger->trigger_id, thaco_external_xyz_trigger->time_boot_ms);
}

/**
 * @brief Encode a thaco_external_xyz_trigger struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param thaco_external_xyz_trigger C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_thaco_external_xyz_trigger_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_thaco_external_xyz_trigger_t* thaco_external_xyz_trigger)
{
    return mavlink_msg_thaco_external_xyz_trigger_pack_chan(system_id, component_id, chan, msg, thaco_external_xyz_trigger->trigger_id, thaco_external_xyz_trigger->time_boot_ms);
}

/**
 * @brief Encode a thaco_external_xyz_trigger struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param thaco_external_xyz_trigger C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_thaco_external_xyz_trigger_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_thaco_external_xyz_trigger_t* thaco_external_xyz_trigger)
{
    return mavlink_msg_thaco_external_xyz_trigger_pack_status(system_id, component_id, _status, msg,  thaco_external_xyz_trigger->trigger_id, thaco_external_xyz_trigger->time_boot_ms);
}

/**
 * @brief Send a thaco_external_xyz_trigger message
 * @param chan MAVLink channel to send the message
 *
 * @param trigger_id  Stable per-handoff trigger ID in range 1..16777216
 * @param time_boot_ms  Timestamp in milliseconds since system boot
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_thaco_external_xyz_trigger_send(mavlink_channel_t chan, uint32_t trigger_id, uint32_t time_boot_ms)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN];
    _mav_put_uint32_t(buf, 0, trigger_id);
    _mav_put_uint32_t(buf, 4, time_boot_ms);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER, buf, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
#else
    mavlink_thaco_external_xyz_trigger_t packet;
    packet.trigger_id = trigger_id;
    packet.time_boot_ms = time_boot_ms;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER, (const char *)&packet, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
#endif
}

/**
 * @brief Send a thaco_external_xyz_trigger message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_thaco_external_xyz_trigger_send_struct(mavlink_channel_t chan, const mavlink_thaco_external_xyz_trigger_t* thaco_external_xyz_trigger)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_thaco_external_xyz_trigger_send(chan, thaco_external_xyz_trigger->trigger_id, thaco_external_xyz_trigger->time_boot_ms);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER, (const char *)thaco_external_xyz_trigger, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
#endif
}

#if MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_thaco_external_xyz_trigger_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t trigger_id, uint32_t time_boot_ms)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, trigger_id);
    _mav_put_uint32_t(buf, 4, time_boot_ms);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER, buf, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
#else
    mavlink_thaco_external_xyz_trigger_t *packet = (mavlink_thaco_external_xyz_trigger_t *)msgbuf;
    packet->trigger_id = trigger_id;
    packet->time_boot_ms = time_boot_ms;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER, (const char *)packet, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_CRC);
#endif
}
#endif

#endif

// MESSAGE THACO_EXTERNAL_XYZ_TRIGGER UNPACKING


/**
 * @brief Get field trigger_id from thaco_external_xyz_trigger message
 *
 * @return  Stable per-handoff trigger ID in range 1..16777216
 */
static inline uint32_t mavlink_msg_thaco_external_xyz_trigger_get_trigger_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field time_boot_ms from thaco_external_xyz_trigger message
 *
 * @return  Timestamp in milliseconds since system boot
 */
static inline uint32_t mavlink_msg_thaco_external_xyz_trigger_get_time_boot_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Decode a thaco_external_xyz_trigger message into a struct
 *
 * @param msg The message to decode
 * @param thaco_external_xyz_trigger C-struct to decode the message contents into
 */
static inline void mavlink_msg_thaco_external_xyz_trigger_decode(const mavlink_message_t* msg, mavlink_thaco_external_xyz_trigger_t* thaco_external_xyz_trigger)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    thaco_external_xyz_trigger->trigger_id = mavlink_msg_thaco_external_xyz_trigger_get_trigger_id(msg);
    thaco_external_xyz_trigger->time_boot_ms = mavlink_msg_thaco_external_xyz_trigger_get_time_boot_ms(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN? msg->len : MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN;
        memset(thaco_external_xyz_trigger, 0, MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_LEN);
    memcpy(thaco_external_xyz_trigger, _MAV_PAYLOAD(msg), len);
#endif
}
