#pragma once
// MESSAGE CC_TELEMETRY_SYSTEM PACKING

#define MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM 42014


typedef struct __mavlink_cc_telemetry_system_t {
 uint32_t system_uptime_s; /*<  System uptime since boot in seconds*/
 uint8_t cpu_usage; /*<  CPU utilization percentage (0 - 100)*/
 uint8_t ram_usage; /*<  RAM memory utilization percentage (0 - 100)*/
 uint8_t disk_usage; /*<  Storage eMMC/SD card utilization percentage (0 - 100)*/
 int8_t cpu_temp; /*<  CPU SoC temperature in degrees Celsius*/
 uint8_t system_status; /*<  System health alerts bitmask*/
} mavlink_cc_telemetry_system_t;

#define MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN 9
#define MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN 9
#define MAVLINK_MSG_ID_42014_LEN 9
#define MAVLINK_MSG_ID_42014_MIN_LEN 9

#define MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC 159
#define MAVLINK_MSG_ID_42014_CRC 159



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_SYSTEM { \
    42014, \
    "CC_TELEMETRY_SYSTEM", \
    6, \
    {  { "system_uptime_s", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_telemetry_system_t, system_uptime_s) }, \
         { "cpu_usage", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_cc_telemetry_system_t, cpu_usage) }, \
         { "ram_usage", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_cc_telemetry_system_t, ram_usage) }, \
         { "disk_usage", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_cc_telemetry_system_t, disk_usage) }, \
         { "cpu_temp", NULL, MAVLINK_TYPE_INT8_T, 0, 7, offsetof(mavlink_cc_telemetry_system_t, cpu_temp) }, \
         { "system_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_cc_telemetry_system_t, system_status) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_SYSTEM { \
    "CC_TELEMETRY_SYSTEM", \
    6, \
    {  { "system_uptime_s", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_telemetry_system_t, system_uptime_s) }, \
         { "cpu_usage", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_cc_telemetry_system_t, cpu_usage) }, \
         { "ram_usage", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_cc_telemetry_system_t, ram_usage) }, \
         { "disk_usage", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_cc_telemetry_system_t, disk_usage) }, \
         { "cpu_temp", NULL, MAVLINK_TYPE_INT8_T, 0, 7, offsetof(mavlink_cc_telemetry_system_t, cpu_temp) }, \
         { "system_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_cc_telemetry_system_t, system_status) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_telemetry_system message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param system_uptime_s  System uptime since boot in seconds
 * @param cpu_usage  CPU utilization percentage (0 - 100)
 * @param ram_usage  RAM memory utilization percentage (0 - 100)
 * @param disk_usage  Storage eMMC/SD card utilization percentage (0 - 100)
 * @param cpu_temp  CPU SoC temperature in degrees Celsius
 * @param system_status  System health alerts bitmask
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_system_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t system_uptime_s, uint8_t cpu_usage, uint8_t ram_usage, uint8_t disk_usage, int8_t cpu_temp, uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN];
    _mav_put_uint32_t(buf, 0, system_uptime_s);
    _mav_put_uint8_t(buf, 4, cpu_usage);
    _mav_put_uint8_t(buf, 5, ram_usage);
    _mav_put_uint8_t(buf, 6, disk_usage);
    _mav_put_int8_t(buf, 7, cpu_temp);
    _mav_put_uint8_t(buf, 8, system_status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#else
    mavlink_cc_telemetry_system_t packet;
    packet.system_uptime_s = system_uptime_s;
    packet.cpu_usage = cpu_usage;
    packet.ram_usage = ram_usage;
    packet.disk_usage = disk_usage;
    packet.cpu_temp = cpu_temp;
    packet.system_status = system_status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
}

/**
 * @brief Pack a cc_telemetry_system message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param system_uptime_s  System uptime since boot in seconds
 * @param cpu_usage  CPU utilization percentage (0 - 100)
 * @param ram_usage  RAM memory utilization percentage (0 - 100)
 * @param disk_usage  Storage eMMC/SD card utilization percentage (0 - 100)
 * @param cpu_temp  CPU SoC temperature in degrees Celsius
 * @param system_status  System health alerts bitmask
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_system_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t system_uptime_s, uint8_t cpu_usage, uint8_t ram_usage, uint8_t disk_usage, int8_t cpu_temp, uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN];
    _mav_put_uint32_t(buf, 0, system_uptime_s);
    _mav_put_uint8_t(buf, 4, cpu_usage);
    _mav_put_uint8_t(buf, 5, ram_usage);
    _mav_put_uint8_t(buf, 6, disk_usage);
    _mav_put_int8_t(buf, 7, cpu_temp);
    _mav_put_uint8_t(buf, 8, system_status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#else
    mavlink_cc_telemetry_system_t packet;
    packet.system_uptime_s = system_uptime_s;
    packet.cpu_usage = cpu_usage;
    packet.ram_usage = ram_usage;
    packet.disk_usage = disk_usage;
    packet.cpu_temp = cpu_temp;
    packet.system_status = system_status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#endif
}

/**
 * @brief Pack a cc_telemetry_system message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param system_uptime_s  System uptime since boot in seconds
 * @param cpu_usage  CPU utilization percentage (0 - 100)
 * @param ram_usage  RAM memory utilization percentage (0 - 100)
 * @param disk_usage  Storage eMMC/SD card utilization percentage (0 - 100)
 * @param cpu_temp  CPU SoC temperature in degrees Celsius
 * @param system_status  System health alerts bitmask
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_system_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t system_uptime_s,uint8_t cpu_usage,uint8_t ram_usage,uint8_t disk_usage,int8_t cpu_temp,uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN];
    _mav_put_uint32_t(buf, 0, system_uptime_s);
    _mav_put_uint8_t(buf, 4, cpu_usage);
    _mav_put_uint8_t(buf, 5, ram_usage);
    _mav_put_uint8_t(buf, 6, disk_usage);
    _mav_put_int8_t(buf, 7, cpu_temp);
    _mav_put_uint8_t(buf, 8, system_status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#else
    mavlink_cc_telemetry_system_t packet;
    packet.system_uptime_s = system_uptime_s;
    packet.cpu_usage = cpu_usage;
    packet.ram_usage = ram_usage;
    packet.disk_usage = disk_usage;
    packet.cpu_temp = cpu_temp;
    packet.system_status = system_status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
}

/**
 * @brief Encode a cc_telemetry_system struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_system C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_system_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_telemetry_system_t* cc_telemetry_system)
{
    return mavlink_msg_cc_telemetry_system_pack(system_id, component_id, msg, cc_telemetry_system->system_uptime_s, cc_telemetry_system->cpu_usage, cc_telemetry_system->ram_usage, cc_telemetry_system->disk_usage, cc_telemetry_system->cpu_temp, cc_telemetry_system->system_status);
}

/**
 * @brief Encode a cc_telemetry_system struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_system C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_system_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_telemetry_system_t* cc_telemetry_system)
{
    return mavlink_msg_cc_telemetry_system_pack_chan(system_id, component_id, chan, msg, cc_telemetry_system->system_uptime_s, cc_telemetry_system->cpu_usage, cc_telemetry_system->ram_usage, cc_telemetry_system->disk_usage, cc_telemetry_system->cpu_temp, cc_telemetry_system->system_status);
}

/**
 * @brief Encode a cc_telemetry_system struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_system C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_system_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_telemetry_system_t* cc_telemetry_system)
{
    return mavlink_msg_cc_telemetry_system_pack_status(system_id, component_id, _status, msg,  cc_telemetry_system->system_uptime_s, cc_telemetry_system->cpu_usage, cc_telemetry_system->ram_usage, cc_telemetry_system->disk_usage, cc_telemetry_system->cpu_temp, cc_telemetry_system->system_status);
}

/**
 * @brief Send a cc_telemetry_system message
 * @param chan MAVLink channel to send the message
 *
 * @param system_uptime_s  System uptime since boot in seconds
 * @param cpu_usage  CPU utilization percentage (0 - 100)
 * @param ram_usage  RAM memory utilization percentage (0 - 100)
 * @param disk_usage  Storage eMMC/SD card utilization percentage (0 - 100)
 * @param cpu_temp  CPU SoC temperature in degrees Celsius
 * @param system_status  System health alerts bitmask
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_telemetry_system_send(mavlink_channel_t chan, uint32_t system_uptime_s, uint8_t cpu_usage, uint8_t ram_usage, uint8_t disk_usage, int8_t cpu_temp, uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN];
    _mav_put_uint32_t(buf, 0, system_uptime_s);
    _mav_put_uint8_t(buf, 4, cpu_usage);
    _mav_put_uint8_t(buf, 5, ram_usage);
    _mav_put_uint8_t(buf, 6, disk_usage);
    _mav_put_int8_t(buf, 7, cpu_temp);
    _mav_put_uint8_t(buf, 8, system_status);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM, buf, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
#else
    mavlink_cc_telemetry_system_t packet;
    packet.system_uptime_s = system_uptime_s;
    packet.cpu_usage = cpu_usage;
    packet.ram_usage = ram_usage;
    packet.disk_usage = disk_usage;
    packet.cpu_temp = cpu_temp;
    packet.system_status = system_status;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM, (const char *)&packet, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
#endif
}

/**
 * @brief Send a cc_telemetry_system message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_telemetry_system_send_struct(mavlink_channel_t chan, const mavlink_cc_telemetry_system_t* cc_telemetry_system)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_telemetry_system_send(chan, cc_telemetry_system->system_uptime_s, cc_telemetry_system->cpu_usage, cc_telemetry_system->ram_usage, cc_telemetry_system->disk_usage, cc_telemetry_system->cpu_temp, cc_telemetry_system->system_status);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM, (const char *)cc_telemetry_system, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_telemetry_system_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t system_uptime_s, uint8_t cpu_usage, uint8_t ram_usage, uint8_t disk_usage, int8_t cpu_temp, uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, system_uptime_s);
    _mav_put_uint8_t(buf, 4, cpu_usage);
    _mav_put_uint8_t(buf, 5, ram_usage);
    _mav_put_uint8_t(buf, 6, disk_usage);
    _mav_put_int8_t(buf, 7, cpu_temp);
    _mav_put_uint8_t(buf, 8, system_status);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM, buf, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
#else
    mavlink_cc_telemetry_system_t *packet = (mavlink_cc_telemetry_system_t *)msgbuf;
    packet->system_uptime_s = system_uptime_s;
    packet->cpu_usage = cpu_usage;
    packet->ram_usage = ram_usage;
    packet->disk_usage = disk_usage;
    packet->cpu_temp = cpu_temp;
    packet->system_status = system_status;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM, (const char *)packet, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_TELEMETRY_SYSTEM UNPACKING


/**
 * @brief Get field system_uptime_s from cc_telemetry_system message
 *
 * @return  System uptime since boot in seconds
 */
static inline uint32_t mavlink_msg_cc_telemetry_system_get_system_uptime_s(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field cpu_usage from cc_telemetry_system message
 *
 * @return  CPU utilization percentage (0 - 100)
 */
static inline uint8_t mavlink_msg_cc_telemetry_system_get_cpu_usage(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field ram_usage from cc_telemetry_system message
 *
 * @return  RAM memory utilization percentage (0 - 100)
 */
static inline uint8_t mavlink_msg_cc_telemetry_system_get_ram_usage(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  5);
}

/**
 * @brief Get field disk_usage from cc_telemetry_system message
 *
 * @return  Storage eMMC/SD card utilization percentage (0 - 100)
 */
static inline uint8_t mavlink_msg_cc_telemetry_system_get_disk_usage(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  6);
}

/**
 * @brief Get field cpu_temp from cc_telemetry_system message
 *
 * @return  CPU SoC temperature in degrees Celsius
 */
static inline int8_t mavlink_msg_cc_telemetry_system_get_cpu_temp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  7);
}

/**
 * @brief Get field system_status from cc_telemetry_system message
 *
 * @return  System health alerts bitmask
 */
static inline uint8_t mavlink_msg_cc_telemetry_system_get_system_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Decode a cc_telemetry_system message into a struct
 *
 * @param msg The message to decode
 * @param cc_telemetry_system C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_telemetry_system_decode(const mavlink_message_t* msg, mavlink_cc_telemetry_system_t* cc_telemetry_system)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_telemetry_system->system_uptime_s = mavlink_msg_cc_telemetry_system_get_system_uptime_s(msg);
    cc_telemetry_system->cpu_usage = mavlink_msg_cc_telemetry_system_get_cpu_usage(msg);
    cc_telemetry_system->ram_usage = mavlink_msg_cc_telemetry_system_get_ram_usage(msg);
    cc_telemetry_system->disk_usage = mavlink_msg_cc_telemetry_system_get_disk_usage(msg);
    cc_telemetry_system->cpu_temp = mavlink_msg_cc_telemetry_system_get_cpu_temp(msg);
    cc_telemetry_system->system_status = mavlink_msg_cc_telemetry_system_get_system_status(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN? msg->len : MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN;
        memset(cc_telemetry_system, 0, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_LEN);
    memcpy(cc_telemetry_system, _MAV_PAYLOAD(msg), len);
#endif
}
