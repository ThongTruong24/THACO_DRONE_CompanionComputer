#pragma once
// MESSAGE CC_TELEMETRY_VISION PACKING

#define MAVLINK_MSG_ID_CC_TELEMETRY_VISION 42013


typedef struct __mavlink_cc_telemetry_vision_t {
 float confidence_thresh; /*<  Detection confidence threshold, e.g. 0.35*/
 float inference_fps; /*<  Current inference frames per second, e.g. 5.0*/
 uint16_t input_width; /*<  Model input width in pixels, e.g. 640*/
 uint16_t input_height; /*<  Model input height in pixels, e.g. 360*/
 uint8_t video_fps; /*<  Camera input FPS, e.g. 20*/
 uint8_t detections_count; /*<  Current count of detected target objects*/
 uint8_t status_flags; /*<  Vision pipeline status: bit 0: running, bit 1: depth enabled*/
 char model_name[24]; /*<  Active YOLO model filename, e.g. yolo26n.pt*/
 char input_source[12]; /*<  Input video source: rtsp, realsense, v4l2*/
} mavlink_cc_telemetry_vision_t;

#define MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN 51
#define MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN 51
#define MAVLINK_MSG_ID_42013_LEN 51
#define MAVLINK_MSG_ID_42013_MIN_LEN 51

#define MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC 234
#define MAVLINK_MSG_ID_42013_CRC 234

#define MAVLINK_MSG_CC_TELEMETRY_VISION_FIELD_MODEL_NAME_LEN 24
#define MAVLINK_MSG_CC_TELEMETRY_VISION_FIELD_INPUT_SOURCE_LEN 12

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_VISION { \
    42013, \
    "CC_TELEMETRY_VISION", \
    9, \
    {  { "confidence_thresh", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_cc_telemetry_vision_t, confidence_thresh) }, \
         { "inference_fps", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_cc_telemetry_vision_t, inference_fps) }, \
         { "input_width", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_cc_telemetry_vision_t, input_width) }, \
         { "input_height", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_cc_telemetry_vision_t, input_height) }, \
         { "video_fps", NULL, MAVLINK_TYPE_UINT8_T, 0, 12, offsetof(mavlink_cc_telemetry_vision_t, video_fps) }, \
         { "detections_count", NULL, MAVLINK_TYPE_UINT8_T, 0, 13, offsetof(mavlink_cc_telemetry_vision_t, detections_count) }, \
         { "status_flags", NULL, MAVLINK_TYPE_UINT8_T, 0, 14, offsetof(mavlink_cc_telemetry_vision_t, status_flags) }, \
         { "model_name", NULL, MAVLINK_TYPE_CHAR, 24, 15, offsetof(mavlink_cc_telemetry_vision_t, model_name) }, \
         { "input_source", NULL, MAVLINK_TYPE_CHAR, 12, 39, offsetof(mavlink_cc_telemetry_vision_t, input_source) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_VISION { \
    "CC_TELEMETRY_VISION", \
    9, \
    {  { "confidence_thresh", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_cc_telemetry_vision_t, confidence_thresh) }, \
         { "inference_fps", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_cc_telemetry_vision_t, inference_fps) }, \
         { "input_width", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_cc_telemetry_vision_t, input_width) }, \
         { "input_height", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_cc_telemetry_vision_t, input_height) }, \
         { "video_fps", NULL, MAVLINK_TYPE_UINT8_T, 0, 12, offsetof(mavlink_cc_telemetry_vision_t, video_fps) }, \
         { "detections_count", NULL, MAVLINK_TYPE_UINT8_T, 0, 13, offsetof(mavlink_cc_telemetry_vision_t, detections_count) }, \
         { "status_flags", NULL, MAVLINK_TYPE_UINT8_T, 0, 14, offsetof(mavlink_cc_telemetry_vision_t, status_flags) }, \
         { "model_name", NULL, MAVLINK_TYPE_CHAR, 24, 15, offsetof(mavlink_cc_telemetry_vision_t, model_name) }, \
         { "input_source", NULL, MAVLINK_TYPE_CHAR, 12, 39, offsetof(mavlink_cc_telemetry_vision_t, input_source) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_telemetry_vision message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param confidence_thresh  Detection confidence threshold, e.g. 0.35
 * @param inference_fps  Current inference frames per second, e.g. 5.0
 * @param input_width  Model input width in pixels, e.g. 640
 * @param input_height  Model input height in pixels, e.g. 360
 * @param video_fps  Camera input FPS, e.g. 20
 * @param detections_count  Current count of detected target objects
 * @param status_flags  Vision pipeline status: bit 0: running, bit 1: depth enabled
 * @param model_name  Active YOLO model filename, e.g. yolo26n.pt
 * @param input_source  Input video source: rtsp, realsense, v4l2
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               float confidence_thresh, float inference_fps, uint16_t input_width, uint16_t input_height, uint8_t video_fps, uint8_t detections_count, uint8_t status_flags, const char *model_name, const char *input_source)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN];
    _mav_put_float(buf, 0, confidence_thresh);
    _mav_put_float(buf, 4, inference_fps);
    _mav_put_uint16_t(buf, 8, input_width);
    _mav_put_uint16_t(buf, 10, input_height);
    _mav_put_uint8_t(buf, 12, video_fps);
    _mav_put_uint8_t(buf, 13, detections_count);
    _mav_put_uint8_t(buf, 14, status_flags);
    _mav_put_char_array(buf, 15, model_name, 24);
    _mav_put_char_array(buf, 39, input_source, 12);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#else
    mavlink_cc_telemetry_vision_t packet;
    packet.confidence_thresh = confidence_thresh;
    packet.inference_fps = inference_fps;
    packet.input_width = input_width;
    packet.input_height = input_height;
    packet.video_fps = video_fps;
    packet.detections_count = detections_count;
    packet.status_flags = status_flags;
    mav_array_assign_char(packet.model_name, model_name, 24);
    mav_array_assign_char(packet.input_source, input_source, 12);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_VISION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
}

/**
 * @brief Pack a cc_telemetry_vision message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param confidence_thresh  Detection confidence threshold, e.g. 0.35
 * @param inference_fps  Current inference frames per second, e.g. 5.0
 * @param input_width  Model input width in pixels, e.g. 640
 * @param input_height  Model input height in pixels, e.g. 360
 * @param video_fps  Camera input FPS, e.g. 20
 * @param detections_count  Current count of detected target objects
 * @param status_flags  Vision pipeline status: bit 0: running, bit 1: depth enabled
 * @param model_name  Active YOLO model filename, e.g. yolo26n.pt
 * @param input_source  Input video source: rtsp, realsense, v4l2
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               float confidence_thresh, float inference_fps, uint16_t input_width, uint16_t input_height, uint8_t video_fps, uint8_t detections_count, uint8_t status_flags, const char *model_name, const char *input_source)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN];
    _mav_put_float(buf, 0, confidence_thresh);
    _mav_put_float(buf, 4, inference_fps);
    _mav_put_uint16_t(buf, 8, input_width);
    _mav_put_uint16_t(buf, 10, input_height);
    _mav_put_uint8_t(buf, 12, video_fps);
    _mav_put_uint8_t(buf, 13, detections_count);
    _mav_put_uint8_t(buf, 14, status_flags);
    _mav_put_char_array(buf, 15, model_name, 24);
    _mav_put_char_array(buf, 39, input_source, 12);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#else
    mavlink_cc_telemetry_vision_t packet;
    packet.confidence_thresh = confidence_thresh;
    packet.inference_fps = inference_fps;
    packet.input_width = input_width;
    packet.input_height = input_height;
    packet.video_fps = video_fps;
    packet.detections_count = detections_count;
    packet.status_flags = status_flags;
    mav_array_memcpy(packet.model_name, model_name, sizeof(char)*24);
    mav_array_memcpy(packet.input_source, input_source, sizeof(char)*12);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_VISION;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#endif
}

/**
 * @brief Pack a cc_telemetry_vision message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param confidence_thresh  Detection confidence threshold, e.g. 0.35
 * @param inference_fps  Current inference frames per second, e.g. 5.0
 * @param input_width  Model input width in pixels, e.g. 640
 * @param input_height  Model input height in pixels, e.g. 360
 * @param video_fps  Camera input FPS, e.g. 20
 * @param detections_count  Current count of detected target objects
 * @param status_flags  Vision pipeline status: bit 0: running, bit 1: depth enabled
 * @param model_name  Active YOLO model filename, e.g. yolo26n.pt
 * @param input_source  Input video source: rtsp, realsense, v4l2
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   float confidence_thresh,float inference_fps,uint16_t input_width,uint16_t input_height,uint8_t video_fps,uint8_t detections_count,uint8_t status_flags,const char *model_name,const char *input_source)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN];
    _mav_put_float(buf, 0, confidence_thresh);
    _mav_put_float(buf, 4, inference_fps);
    _mav_put_uint16_t(buf, 8, input_width);
    _mav_put_uint16_t(buf, 10, input_height);
    _mav_put_uint8_t(buf, 12, video_fps);
    _mav_put_uint8_t(buf, 13, detections_count);
    _mav_put_uint8_t(buf, 14, status_flags);
    _mav_put_char_array(buf, 15, model_name, 24);
    _mav_put_char_array(buf, 39, input_source, 12);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#else
    mavlink_cc_telemetry_vision_t packet;
    packet.confidence_thresh = confidence_thresh;
    packet.inference_fps = inference_fps;
    packet.input_width = input_width;
    packet.input_height = input_height;
    packet.video_fps = video_fps;
    packet.detections_count = detections_count;
    packet.status_flags = status_flags;
    mav_array_assign_char(packet.model_name, model_name, 24);
    mav_array_assign_char(packet.input_source, input_source, 12);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_VISION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
}

/**
 * @brief Encode a cc_telemetry_vision struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_vision C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_telemetry_vision_t* cc_telemetry_vision)
{
    return mavlink_msg_cc_telemetry_vision_pack(system_id, component_id, msg, cc_telemetry_vision->confidence_thresh, cc_telemetry_vision->inference_fps, cc_telemetry_vision->input_width, cc_telemetry_vision->input_height, cc_telemetry_vision->video_fps, cc_telemetry_vision->detections_count, cc_telemetry_vision->status_flags, cc_telemetry_vision->model_name, cc_telemetry_vision->input_source);
}

/**
 * @brief Encode a cc_telemetry_vision struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_vision C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_telemetry_vision_t* cc_telemetry_vision)
{
    return mavlink_msg_cc_telemetry_vision_pack_chan(system_id, component_id, chan, msg, cc_telemetry_vision->confidence_thresh, cc_telemetry_vision->inference_fps, cc_telemetry_vision->input_width, cc_telemetry_vision->input_height, cc_telemetry_vision->video_fps, cc_telemetry_vision->detections_count, cc_telemetry_vision->status_flags, cc_telemetry_vision->model_name, cc_telemetry_vision->input_source);
}

/**
 * @brief Encode a cc_telemetry_vision struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_vision C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_telemetry_vision_t* cc_telemetry_vision)
{
    return mavlink_msg_cc_telemetry_vision_pack_status(system_id, component_id, _status, msg,  cc_telemetry_vision->confidence_thresh, cc_telemetry_vision->inference_fps, cc_telemetry_vision->input_width, cc_telemetry_vision->input_height, cc_telemetry_vision->video_fps, cc_telemetry_vision->detections_count, cc_telemetry_vision->status_flags, cc_telemetry_vision->model_name, cc_telemetry_vision->input_source);
}

/**
 * @brief Send a cc_telemetry_vision message
 * @param chan MAVLink channel to send the message
 *
 * @param confidence_thresh  Detection confidence threshold, e.g. 0.35
 * @param inference_fps  Current inference frames per second, e.g. 5.0
 * @param input_width  Model input width in pixels, e.g. 640
 * @param input_height  Model input height in pixels, e.g. 360
 * @param video_fps  Camera input FPS, e.g. 20
 * @param detections_count  Current count of detected target objects
 * @param status_flags  Vision pipeline status: bit 0: running, bit 1: depth enabled
 * @param model_name  Active YOLO model filename, e.g. yolo26n.pt
 * @param input_source  Input video source: rtsp, realsense, v4l2
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_telemetry_vision_send(mavlink_channel_t chan, float confidence_thresh, float inference_fps, uint16_t input_width, uint16_t input_height, uint8_t video_fps, uint8_t detections_count, uint8_t status_flags, const char *model_name, const char *input_source)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN];
    _mav_put_float(buf, 0, confidence_thresh);
    _mav_put_float(buf, 4, inference_fps);
    _mav_put_uint16_t(buf, 8, input_width);
    _mav_put_uint16_t(buf, 10, input_height);
    _mav_put_uint8_t(buf, 12, video_fps);
    _mav_put_uint8_t(buf, 13, detections_count);
    _mav_put_uint8_t(buf, 14, status_flags);
    _mav_put_char_array(buf, 15, model_name, 24);
    _mav_put_char_array(buf, 39, input_source, 12);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_VISION, buf, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
#else
    mavlink_cc_telemetry_vision_t packet;
    packet.confidence_thresh = confidence_thresh;
    packet.inference_fps = inference_fps;
    packet.input_width = input_width;
    packet.input_height = input_height;
    packet.video_fps = video_fps;
    packet.detections_count = detections_count;
    packet.status_flags = status_flags;
    mav_array_assign_char(packet.model_name, model_name, 24);
    mav_array_assign_char(packet.input_source, input_source, 12);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_VISION, (const char *)&packet, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
#endif
}

/**
 * @brief Send a cc_telemetry_vision message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_telemetry_vision_send_struct(mavlink_channel_t chan, const mavlink_cc_telemetry_vision_t* cc_telemetry_vision)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_telemetry_vision_send(chan, cc_telemetry_vision->confidence_thresh, cc_telemetry_vision->inference_fps, cc_telemetry_vision->input_width, cc_telemetry_vision->input_height, cc_telemetry_vision->video_fps, cc_telemetry_vision->detections_count, cc_telemetry_vision->status_flags, cc_telemetry_vision->model_name, cc_telemetry_vision->input_source);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_VISION, (const char *)cc_telemetry_vision, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_telemetry_vision_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  float confidence_thresh, float inference_fps, uint16_t input_width, uint16_t input_height, uint8_t video_fps, uint8_t detections_count, uint8_t status_flags, const char *model_name, const char *input_source)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, confidence_thresh);
    _mav_put_float(buf, 4, inference_fps);
    _mav_put_uint16_t(buf, 8, input_width);
    _mav_put_uint16_t(buf, 10, input_height);
    _mav_put_uint8_t(buf, 12, video_fps);
    _mav_put_uint8_t(buf, 13, detections_count);
    _mav_put_uint8_t(buf, 14, status_flags);
    _mav_put_char_array(buf, 15, model_name, 24);
    _mav_put_char_array(buf, 39, input_source, 12);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_VISION, buf, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
#else
    mavlink_cc_telemetry_vision_t *packet = (mavlink_cc_telemetry_vision_t *)msgbuf;
    packet->confidence_thresh = confidence_thresh;
    packet->inference_fps = inference_fps;
    packet->input_width = input_width;
    packet->input_height = input_height;
    packet->video_fps = video_fps;
    packet->detections_count = detections_count;
    packet->status_flags = status_flags;
    mav_array_assign_char(packet->model_name, model_name, 24);
    mav_array_assign_char(packet->input_source, input_source, 12);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_VISION, (const char *)packet, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_TELEMETRY_VISION UNPACKING


/**
 * @brief Get field confidence_thresh from cc_telemetry_vision message
 *
 * @return  Detection confidence threshold, e.g. 0.35
 */
static inline float mavlink_msg_cc_telemetry_vision_get_confidence_thresh(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field inference_fps from cc_telemetry_vision message
 *
 * @return  Current inference frames per second, e.g. 5.0
 */
static inline float mavlink_msg_cc_telemetry_vision_get_inference_fps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field input_width from cc_telemetry_vision message
 *
 * @return  Model input width in pixels, e.g. 640
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_get_input_width(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  8);
}

/**
 * @brief Get field input_height from cc_telemetry_vision message
 *
 * @return  Model input height in pixels, e.g. 360
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_get_input_height(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  10);
}

/**
 * @brief Get field video_fps from cc_telemetry_vision message
 *
 * @return  Camera input FPS, e.g. 20
 */
static inline uint8_t mavlink_msg_cc_telemetry_vision_get_video_fps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  12);
}

/**
 * @brief Get field detections_count from cc_telemetry_vision message
 *
 * @return  Current count of detected target objects
 */
static inline uint8_t mavlink_msg_cc_telemetry_vision_get_detections_count(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  13);
}

/**
 * @brief Get field status_flags from cc_telemetry_vision message
 *
 * @return  Vision pipeline status: bit 0: running, bit 1: depth enabled
 */
static inline uint8_t mavlink_msg_cc_telemetry_vision_get_status_flags(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  14);
}

/**
 * @brief Get field model_name from cc_telemetry_vision message
 *
 * @return  Active YOLO model filename, e.g. yolo26n.pt
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_get_model_name(const mavlink_message_t* msg, char *model_name)
{
    return _MAV_RETURN_char_array(msg, model_name, 24,  15);
}

/**
 * @brief Get field input_source from cc_telemetry_vision message
 *
 * @return  Input video source: rtsp, realsense, v4l2
 */
static inline uint16_t mavlink_msg_cc_telemetry_vision_get_input_source(const mavlink_message_t* msg, char *input_source)
{
    return _MAV_RETURN_char_array(msg, input_source, 12,  39);
}

/**
 * @brief Decode a cc_telemetry_vision message into a struct
 *
 * @param msg The message to decode
 * @param cc_telemetry_vision C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_telemetry_vision_decode(const mavlink_message_t* msg, mavlink_cc_telemetry_vision_t* cc_telemetry_vision)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_telemetry_vision->confidence_thresh = mavlink_msg_cc_telemetry_vision_get_confidence_thresh(msg);
    cc_telemetry_vision->inference_fps = mavlink_msg_cc_telemetry_vision_get_inference_fps(msg);
    cc_telemetry_vision->input_width = mavlink_msg_cc_telemetry_vision_get_input_width(msg);
    cc_telemetry_vision->input_height = mavlink_msg_cc_telemetry_vision_get_input_height(msg);
    cc_telemetry_vision->video_fps = mavlink_msg_cc_telemetry_vision_get_video_fps(msg);
    cc_telemetry_vision->detections_count = mavlink_msg_cc_telemetry_vision_get_detections_count(msg);
    cc_telemetry_vision->status_flags = mavlink_msg_cc_telemetry_vision_get_status_flags(msg);
    mavlink_msg_cc_telemetry_vision_get_model_name(msg, cc_telemetry_vision->model_name);
    mavlink_msg_cc_telemetry_vision_get_input_source(msg, cc_telemetry_vision->input_source);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN? msg->len : MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN;
        memset(cc_telemetry_vision, 0, MAVLINK_MSG_ID_CC_TELEMETRY_VISION_LEN);
    memcpy(cc_telemetry_vision, _MAV_PAYLOAD(msg), len);
#endif
}
