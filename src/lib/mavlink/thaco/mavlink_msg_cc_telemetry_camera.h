#pragma once
// MESSAGE CC_TELEMETRY_CAMERA PACKING

#define MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA 42011


typedef struct __mavlink_cc_telemetry_camera_t {
 uint16_t video_width; /*<  Video stream resolution width in pixels, e.g. 1280 or 1920*/
 uint16_t video_height; /*<  Video stream resolution height in pixels, e.g. 720 or 1080*/
 uint16_t depth_width; /*<  Depth stream resolution width in pixels, e.g. 640*/
 uint16_t depth_height; /*<  Depth stream resolution height in pixels, e.g. 480*/
 uint16_t rotation; /*<  Camera sensor rotation in degrees: 0, 90, 180, 270*/
 uint16_t bitrate_kbps; /*<  Encoder nominal target bitrate in kbps, e.g. 2500*/
 uint16_t bitrate_max_kbps; /*<  Encoder maximum peak bitrate in kbps, e.g. 3000*/
 uint16_t vbv_buffer_kb; /*<  Encoder VBV buffer size in kilobits, e.g. 1250*/
 uint16_t rtsp_port; /*<  RTSP streaming server network port, e.g. 8554*/
 uint8_t video_fps; /*<  Video stream framerate, e.g. 30*/
 uint8_t depth_fps; /*<  Depth sensor stream framerate, e.g. 30*/
 uint8_t camera_id; /*<  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV)*/
 uint8_t is_default; /*<  Configuration default persistence flag (1: saved as default, 0: pending/modified)*/
 uint8_t profile_mode; /*<  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud*/
 uint8_t enable_emitter; /*<  Laser projector emitter flag (0: off, 1: on)*/
 uint8_t camera_status; /*<  Active camera operational state*/
 uint8_t error_code; /*<  Specific diagnostic error code*/
 uint8_t usb_speed_mode; /*<  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2*/
 char camera_name[24]; /*<  Human-readable camera model, e.g. Intel RealSense D435i*/
 char camera_type[12]; /*<  Driver type: realsense, v4l2, siyi, csi*/
 char connection_port[20]; /*<  Hardware bus or port identifier, e.g. USB 3.2 Gen 1*/
 char serial_number[16]; /*<  Camera hardware serial number*/
 char codec[6]; /*<  Video codec: h264, h265*/
 char encoder_mode[6]; /*<  Encoder mode: hw, cpu, auto*/
 char rtsp_url_qgc[48]; /*<  RTSP URL endpoint for QGroundControl (AP subnet)*/
 char rtsp_url_controller[48]; /*<  RTSP URL endpoint for Handheld Controller / SIYI link*/
 char rtsp_url_laptop[40]; /*<  RTSP URL endpoint for Laptop / LAN client*/
} mavlink_cc_telemetry_camera_t;

#define MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN 247
#define MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN 247
#define MAVLINK_MSG_ID_42011_LEN 247
#define MAVLINK_MSG_ID_42011_MIN_LEN 247

#define MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC 169
#define MAVLINK_MSG_ID_42011_CRC 169

#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_CAMERA_NAME_LEN 24
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_CAMERA_TYPE_LEN 12
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_CONNECTION_PORT_LEN 20
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_SERIAL_NUMBER_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_CODEC_LEN 6
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_ENCODER_MODE_LEN 6
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_RTSP_URL_QGC_LEN 48
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_RTSP_URL_CONTROLLER_LEN 48
#define MAVLINK_MSG_CC_TELEMETRY_CAMERA_FIELD_RTSP_URL_LAPTOP_LEN 40

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_CAMERA { \
    42011, \
    "CC_TELEMETRY_CAMERA", \
    27, \
    {  { "video_width", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_cc_telemetry_camera_t, video_width) }, \
         { "video_height", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_cc_telemetry_camera_t, video_height) }, \
         { "depth_width", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_cc_telemetry_camera_t, depth_width) }, \
         { "depth_height", NULL, MAVLINK_TYPE_UINT16_T, 0, 6, offsetof(mavlink_cc_telemetry_camera_t, depth_height) }, \
         { "rotation", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_cc_telemetry_camera_t, rotation) }, \
         { "bitrate_kbps", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_cc_telemetry_camera_t, bitrate_kbps) }, \
         { "bitrate_max_kbps", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_cc_telemetry_camera_t, bitrate_max_kbps) }, \
         { "vbv_buffer_kb", NULL, MAVLINK_TYPE_UINT16_T, 0, 14, offsetof(mavlink_cc_telemetry_camera_t, vbv_buffer_kb) }, \
         { "rtsp_port", NULL, MAVLINK_TYPE_UINT16_T, 0, 16, offsetof(mavlink_cc_telemetry_camera_t, rtsp_port) }, \
         { "video_fps", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_cc_telemetry_camera_t, video_fps) }, \
         { "depth_fps", NULL, MAVLINK_TYPE_UINT8_T, 0, 19, offsetof(mavlink_cc_telemetry_camera_t, depth_fps) }, \
         { "camera_id", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_cc_telemetry_camera_t, camera_id) }, \
         { "is_default", NULL, MAVLINK_TYPE_UINT8_T, 0, 21, offsetof(mavlink_cc_telemetry_camera_t, is_default) }, \
         { "profile_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 22, offsetof(mavlink_cc_telemetry_camera_t, profile_mode) }, \
         { "enable_emitter", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_cc_telemetry_camera_t, enable_emitter) }, \
         { "camera_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 24, offsetof(mavlink_cc_telemetry_camera_t, camera_status) }, \
         { "error_code", NULL, MAVLINK_TYPE_UINT8_T, 0, 25, offsetof(mavlink_cc_telemetry_camera_t, error_code) }, \
         { "usb_speed_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 26, offsetof(mavlink_cc_telemetry_camera_t, usb_speed_mode) }, \
         { "camera_name", NULL, MAVLINK_TYPE_CHAR, 24, 27, offsetof(mavlink_cc_telemetry_camera_t, camera_name) }, \
         { "camera_type", NULL, MAVLINK_TYPE_CHAR, 12, 51, offsetof(mavlink_cc_telemetry_camera_t, camera_type) }, \
         { "connection_port", NULL, MAVLINK_TYPE_CHAR, 20, 63, offsetof(mavlink_cc_telemetry_camera_t, connection_port) }, \
         { "serial_number", NULL, MAVLINK_TYPE_CHAR, 16, 83, offsetof(mavlink_cc_telemetry_camera_t, serial_number) }, \
         { "codec", NULL, MAVLINK_TYPE_CHAR, 6, 99, offsetof(mavlink_cc_telemetry_camera_t, codec) }, \
         { "encoder_mode", NULL, MAVLINK_TYPE_CHAR, 6, 105, offsetof(mavlink_cc_telemetry_camera_t, encoder_mode) }, \
         { "rtsp_url_qgc", NULL, MAVLINK_TYPE_CHAR, 48, 111, offsetof(mavlink_cc_telemetry_camera_t, rtsp_url_qgc) }, \
         { "rtsp_url_controller", NULL, MAVLINK_TYPE_CHAR, 48, 159, offsetof(mavlink_cc_telemetry_camera_t, rtsp_url_controller) }, \
         { "rtsp_url_laptop", NULL, MAVLINK_TYPE_CHAR, 40, 207, offsetof(mavlink_cc_telemetry_camera_t, rtsp_url_laptop) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_CAMERA { \
    "CC_TELEMETRY_CAMERA", \
    27, \
    {  { "video_width", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_cc_telemetry_camera_t, video_width) }, \
         { "video_height", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_cc_telemetry_camera_t, video_height) }, \
         { "depth_width", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_cc_telemetry_camera_t, depth_width) }, \
         { "depth_height", NULL, MAVLINK_TYPE_UINT16_T, 0, 6, offsetof(mavlink_cc_telemetry_camera_t, depth_height) }, \
         { "rotation", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_cc_telemetry_camera_t, rotation) }, \
         { "bitrate_kbps", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_cc_telemetry_camera_t, bitrate_kbps) }, \
         { "bitrate_max_kbps", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_cc_telemetry_camera_t, bitrate_max_kbps) }, \
         { "vbv_buffer_kb", NULL, MAVLINK_TYPE_UINT16_T, 0, 14, offsetof(mavlink_cc_telemetry_camera_t, vbv_buffer_kb) }, \
         { "rtsp_port", NULL, MAVLINK_TYPE_UINT16_T, 0, 16, offsetof(mavlink_cc_telemetry_camera_t, rtsp_port) }, \
         { "video_fps", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_cc_telemetry_camera_t, video_fps) }, \
         { "depth_fps", NULL, MAVLINK_TYPE_UINT8_T, 0, 19, offsetof(mavlink_cc_telemetry_camera_t, depth_fps) }, \
         { "camera_id", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_cc_telemetry_camera_t, camera_id) }, \
         { "is_default", NULL, MAVLINK_TYPE_UINT8_T, 0, 21, offsetof(mavlink_cc_telemetry_camera_t, is_default) }, \
         { "profile_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 22, offsetof(mavlink_cc_telemetry_camera_t, profile_mode) }, \
         { "enable_emitter", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_cc_telemetry_camera_t, enable_emitter) }, \
         { "camera_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 24, offsetof(mavlink_cc_telemetry_camera_t, camera_status) }, \
         { "error_code", NULL, MAVLINK_TYPE_UINT8_T, 0, 25, offsetof(mavlink_cc_telemetry_camera_t, error_code) }, \
         { "usb_speed_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 26, offsetof(mavlink_cc_telemetry_camera_t, usb_speed_mode) }, \
         { "camera_name", NULL, MAVLINK_TYPE_CHAR, 24, 27, offsetof(mavlink_cc_telemetry_camera_t, camera_name) }, \
         { "camera_type", NULL, MAVLINK_TYPE_CHAR, 12, 51, offsetof(mavlink_cc_telemetry_camera_t, camera_type) }, \
         { "connection_port", NULL, MAVLINK_TYPE_CHAR, 20, 63, offsetof(mavlink_cc_telemetry_camera_t, connection_port) }, \
         { "serial_number", NULL, MAVLINK_TYPE_CHAR, 16, 83, offsetof(mavlink_cc_telemetry_camera_t, serial_number) }, \
         { "codec", NULL, MAVLINK_TYPE_CHAR, 6, 99, offsetof(mavlink_cc_telemetry_camera_t, codec) }, \
         { "encoder_mode", NULL, MAVLINK_TYPE_CHAR, 6, 105, offsetof(mavlink_cc_telemetry_camera_t, encoder_mode) }, \
         { "rtsp_url_qgc", NULL, MAVLINK_TYPE_CHAR, 48, 111, offsetof(mavlink_cc_telemetry_camera_t, rtsp_url_qgc) }, \
         { "rtsp_url_controller", NULL, MAVLINK_TYPE_CHAR, 48, 159, offsetof(mavlink_cc_telemetry_camera_t, rtsp_url_controller) }, \
         { "rtsp_url_laptop", NULL, MAVLINK_TYPE_CHAR, 40, 207, offsetof(mavlink_cc_telemetry_camera_t, rtsp_url_laptop) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_telemetry_camera message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param video_width  Video stream resolution width in pixels, e.g. 1280 or 1920
 * @param video_height  Video stream resolution height in pixels, e.g. 720 or 1080
 * @param depth_width  Depth stream resolution width in pixels, e.g. 640
 * @param depth_height  Depth stream resolution height in pixels, e.g. 480
 * @param rotation  Camera sensor rotation in degrees: 0, 90, 180, 270
 * @param bitrate_kbps  Encoder nominal target bitrate in kbps, e.g. 2500
 * @param bitrate_max_kbps  Encoder maximum peak bitrate in kbps, e.g. 3000
 * @param vbv_buffer_kb  Encoder VBV buffer size in kilobits, e.g. 1250
 * @param rtsp_port  RTSP streaming server network port, e.g. 8554
 * @param video_fps  Video stream framerate, e.g. 30
 * @param depth_fps  Depth sensor stream framerate, e.g. 30
 * @param camera_id  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV)
 * @param is_default  Configuration default persistence flag (1: saved as default, 0: pending/modified)
 * @param profile_mode  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud
 * @param enable_emitter  Laser projector emitter flag (0: off, 1: on)
 * @param camera_status  Active camera operational state
 * @param error_code  Specific diagnostic error code
 * @param usb_speed_mode  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2
 * @param camera_name  Human-readable camera model, e.g. Intel RealSense D435i
 * @param camera_type  Driver type: realsense, v4l2, siyi, csi
 * @param connection_port  Hardware bus or port identifier, e.g. USB 3.2 Gen 1
 * @param serial_number  Camera hardware serial number
 * @param codec  Video codec: h264, h265
 * @param encoder_mode  Encoder mode: hw, cpu, auto
 * @param rtsp_url_qgc  RTSP URL endpoint for QGroundControl (AP subnet)
 * @param rtsp_url_controller  RTSP URL endpoint for Handheld Controller / SIYI link
 * @param rtsp_url_laptop  RTSP URL endpoint for Laptop / LAN client
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint16_t video_width, uint16_t video_height, uint16_t depth_width, uint16_t depth_height, uint16_t rotation, uint16_t bitrate_kbps, uint16_t bitrate_max_kbps, uint16_t vbv_buffer_kb, uint16_t rtsp_port, uint8_t video_fps, uint8_t depth_fps, uint8_t camera_id, uint8_t is_default, uint8_t profile_mode, uint8_t enable_emitter, uint8_t camera_status, uint8_t error_code, uint8_t usb_speed_mode, const char *camera_name, const char *camera_type, const char *connection_port, const char *serial_number, const char *codec, const char *encoder_mode, const char *rtsp_url_qgc, const char *rtsp_url_controller, const char *rtsp_url_laptop)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN];
    _mav_put_uint16_t(buf, 0, video_width);
    _mav_put_uint16_t(buf, 2, video_height);
    _mav_put_uint16_t(buf, 4, depth_width);
    _mav_put_uint16_t(buf, 6, depth_height);
    _mav_put_uint16_t(buf, 8, rotation);
    _mav_put_uint16_t(buf, 10, bitrate_kbps);
    _mav_put_uint16_t(buf, 12, bitrate_max_kbps);
    _mav_put_uint16_t(buf, 14, vbv_buffer_kb);
    _mav_put_uint16_t(buf, 16, rtsp_port);
    _mav_put_uint8_t(buf, 18, video_fps);
    _mav_put_uint8_t(buf, 19, depth_fps);
    _mav_put_uint8_t(buf, 20, camera_id);
    _mav_put_uint8_t(buf, 21, is_default);
    _mav_put_uint8_t(buf, 22, profile_mode);
    _mav_put_uint8_t(buf, 23, enable_emitter);
    _mav_put_uint8_t(buf, 24, camera_status);
    _mav_put_uint8_t(buf, 25, error_code);
    _mav_put_uint8_t(buf, 26, usb_speed_mode);
    _mav_put_char_array(buf, 27, camera_name, 24);
    _mav_put_char_array(buf, 51, camera_type, 12);
    _mav_put_char_array(buf, 63, connection_port, 20);
    _mav_put_char_array(buf, 83, serial_number, 16);
    _mav_put_char_array(buf, 99, codec, 6);
    _mav_put_char_array(buf, 105, encoder_mode, 6);
    _mav_put_char_array(buf, 111, rtsp_url_qgc, 48);
    _mav_put_char_array(buf, 159, rtsp_url_controller, 48);
    _mav_put_char_array(buf, 207, rtsp_url_laptop, 40);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#else
    mavlink_cc_telemetry_camera_t packet;
    packet.video_width = video_width;
    packet.video_height = video_height;
    packet.depth_width = depth_width;
    packet.depth_height = depth_height;
    packet.rotation = rotation;
    packet.bitrate_kbps = bitrate_kbps;
    packet.bitrate_max_kbps = bitrate_max_kbps;
    packet.vbv_buffer_kb = vbv_buffer_kb;
    packet.rtsp_port = rtsp_port;
    packet.video_fps = video_fps;
    packet.depth_fps = depth_fps;
    packet.camera_id = camera_id;
    packet.is_default = is_default;
    packet.profile_mode = profile_mode;
    packet.enable_emitter = enable_emitter;
    packet.camera_status = camera_status;
    packet.error_code = error_code;
    packet.usb_speed_mode = usb_speed_mode;
    mav_array_memcpy(packet.camera_name, camera_name, sizeof(char)*24);
    mav_array_memcpy(packet.camera_type, camera_type, sizeof(char)*12);
    mav_array_memcpy(packet.connection_port, connection_port, sizeof(char)*20);
    mav_array_memcpy(packet.serial_number, serial_number, sizeof(char)*16);
    mav_array_memcpy(packet.codec, codec, sizeof(char)*6);
    mav_array_memcpy(packet.encoder_mode, encoder_mode, sizeof(char)*6);
    mav_array_memcpy(packet.rtsp_url_qgc, rtsp_url_qgc, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_controller, rtsp_url_controller, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_laptop, rtsp_url_laptop, sizeof(char)*40);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
}

/**
 * @brief Pack a cc_telemetry_camera message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param video_width  Video stream resolution width in pixels, e.g. 1280 or 1920
 * @param video_height  Video stream resolution height in pixels, e.g. 720 or 1080
 * @param depth_width  Depth stream resolution width in pixels, e.g. 640
 * @param depth_height  Depth stream resolution height in pixels, e.g. 480
 * @param rotation  Camera sensor rotation in degrees: 0, 90, 180, 270
 * @param bitrate_kbps  Encoder nominal target bitrate in kbps, e.g. 2500
 * @param bitrate_max_kbps  Encoder maximum peak bitrate in kbps, e.g. 3000
 * @param vbv_buffer_kb  Encoder VBV buffer size in kilobits, e.g. 1250
 * @param rtsp_port  RTSP streaming server network port, e.g. 8554
 * @param video_fps  Video stream framerate, e.g. 30
 * @param depth_fps  Depth sensor stream framerate, e.g. 30
 * @param camera_id  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV)
 * @param is_default  Configuration default persistence flag (1: saved as default, 0: pending/modified)
 * @param profile_mode  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud
 * @param enable_emitter  Laser projector emitter flag (0: off, 1: on)
 * @param camera_status  Active camera operational state
 * @param error_code  Specific diagnostic error code
 * @param usb_speed_mode  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2
 * @param camera_name  Human-readable camera model, e.g. Intel RealSense D435i
 * @param camera_type  Driver type: realsense, v4l2, siyi, csi
 * @param connection_port  Hardware bus or port identifier, e.g. USB 3.2 Gen 1
 * @param serial_number  Camera hardware serial number
 * @param codec  Video codec: h264, h265
 * @param encoder_mode  Encoder mode: hw, cpu, auto
 * @param rtsp_url_qgc  RTSP URL endpoint for QGroundControl (AP subnet)
 * @param rtsp_url_controller  RTSP URL endpoint for Handheld Controller / SIYI link
 * @param rtsp_url_laptop  RTSP URL endpoint for Laptop / LAN client
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint16_t video_width, uint16_t video_height, uint16_t depth_width, uint16_t depth_height, uint16_t rotation, uint16_t bitrate_kbps, uint16_t bitrate_max_kbps, uint16_t vbv_buffer_kb, uint16_t rtsp_port, uint8_t video_fps, uint8_t depth_fps, uint8_t camera_id, uint8_t is_default, uint8_t profile_mode, uint8_t enable_emitter, uint8_t camera_status, uint8_t error_code, uint8_t usb_speed_mode, const char *camera_name, const char *camera_type, const char *connection_port, const char *serial_number, const char *codec, const char *encoder_mode, const char *rtsp_url_qgc, const char *rtsp_url_controller, const char *rtsp_url_laptop)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN];
    _mav_put_uint16_t(buf, 0, video_width);
    _mav_put_uint16_t(buf, 2, video_height);
    _mav_put_uint16_t(buf, 4, depth_width);
    _mav_put_uint16_t(buf, 6, depth_height);
    _mav_put_uint16_t(buf, 8, rotation);
    _mav_put_uint16_t(buf, 10, bitrate_kbps);
    _mav_put_uint16_t(buf, 12, bitrate_max_kbps);
    _mav_put_uint16_t(buf, 14, vbv_buffer_kb);
    _mav_put_uint16_t(buf, 16, rtsp_port);
    _mav_put_uint8_t(buf, 18, video_fps);
    _mav_put_uint8_t(buf, 19, depth_fps);
    _mav_put_uint8_t(buf, 20, camera_id);
    _mav_put_uint8_t(buf, 21, is_default);
    _mav_put_uint8_t(buf, 22, profile_mode);
    _mav_put_uint8_t(buf, 23, enable_emitter);
    _mav_put_uint8_t(buf, 24, camera_status);
    _mav_put_uint8_t(buf, 25, error_code);
    _mav_put_uint8_t(buf, 26, usb_speed_mode);
    _mav_put_char_array(buf, 27, camera_name, 24);
    _mav_put_char_array(buf, 51, camera_type, 12);
    _mav_put_char_array(buf, 63, connection_port, 20);
    _mav_put_char_array(buf, 83, serial_number, 16);
    _mav_put_char_array(buf, 99, codec, 6);
    _mav_put_char_array(buf, 105, encoder_mode, 6);
    _mav_put_char_array(buf, 111, rtsp_url_qgc, 48);
    _mav_put_char_array(buf, 159, rtsp_url_controller, 48);
    _mav_put_char_array(buf, 207, rtsp_url_laptop, 40);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#else
    mavlink_cc_telemetry_camera_t packet;
    packet.video_width = video_width;
    packet.video_height = video_height;
    packet.depth_width = depth_width;
    packet.depth_height = depth_height;
    packet.rotation = rotation;
    packet.bitrate_kbps = bitrate_kbps;
    packet.bitrate_max_kbps = bitrate_max_kbps;
    packet.vbv_buffer_kb = vbv_buffer_kb;
    packet.rtsp_port = rtsp_port;
    packet.video_fps = video_fps;
    packet.depth_fps = depth_fps;
    packet.camera_id = camera_id;
    packet.is_default = is_default;
    packet.profile_mode = profile_mode;
    packet.enable_emitter = enable_emitter;
    packet.camera_status = camera_status;
    packet.error_code = error_code;
    packet.usb_speed_mode = usb_speed_mode;
    mav_array_memcpy(packet.camera_name, camera_name, sizeof(char)*24);
    mav_array_memcpy(packet.camera_type, camera_type, sizeof(char)*12);
    mav_array_memcpy(packet.connection_port, connection_port, sizeof(char)*20);
    mav_array_memcpy(packet.serial_number, serial_number, sizeof(char)*16);
    mav_array_memcpy(packet.codec, codec, sizeof(char)*6);
    mav_array_memcpy(packet.encoder_mode, encoder_mode, sizeof(char)*6);
    mav_array_memcpy(packet.rtsp_url_qgc, rtsp_url_qgc, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_controller, rtsp_url_controller, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_laptop, rtsp_url_laptop, sizeof(char)*40);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#endif
}

/**
 * @brief Pack a cc_telemetry_camera message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param video_width  Video stream resolution width in pixels, e.g. 1280 or 1920
 * @param video_height  Video stream resolution height in pixels, e.g. 720 or 1080
 * @param depth_width  Depth stream resolution width in pixels, e.g. 640
 * @param depth_height  Depth stream resolution height in pixels, e.g. 480
 * @param rotation  Camera sensor rotation in degrees: 0, 90, 180, 270
 * @param bitrate_kbps  Encoder nominal target bitrate in kbps, e.g. 2500
 * @param bitrate_max_kbps  Encoder maximum peak bitrate in kbps, e.g. 3000
 * @param vbv_buffer_kb  Encoder VBV buffer size in kilobits, e.g. 1250
 * @param rtsp_port  RTSP streaming server network port, e.g. 8554
 * @param video_fps  Video stream framerate, e.g. 30
 * @param depth_fps  Depth sensor stream framerate, e.g. 30
 * @param camera_id  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV)
 * @param is_default  Configuration default persistence flag (1: saved as default, 0: pending/modified)
 * @param profile_mode  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud
 * @param enable_emitter  Laser projector emitter flag (0: off, 1: on)
 * @param camera_status  Active camera operational state
 * @param error_code  Specific diagnostic error code
 * @param usb_speed_mode  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2
 * @param camera_name  Human-readable camera model, e.g. Intel RealSense D435i
 * @param camera_type  Driver type: realsense, v4l2, siyi, csi
 * @param connection_port  Hardware bus or port identifier, e.g. USB 3.2 Gen 1
 * @param serial_number  Camera hardware serial number
 * @param codec  Video codec: h264, h265
 * @param encoder_mode  Encoder mode: hw, cpu, auto
 * @param rtsp_url_qgc  RTSP URL endpoint for QGroundControl (AP subnet)
 * @param rtsp_url_controller  RTSP URL endpoint for Handheld Controller / SIYI link
 * @param rtsp_url_laptop  RTSP URL endpoint for Laptop / LAN client
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint16_t video_width,uint16_t video_height,uint16_t depth_width,uint16_t depth_height,uint16_t rotation,uint16_t bitrate_kbps,uint16_t bitrate_max_kbps,uint16_t vbv_buffer_kb,uint16_t rtsp_port,uint8_t video_fps,uint8_t depth_fps,uint8_t camera_id,uint8_t is_default,uint8_t profile_mode,uint8_t enable_emitter,uint8_t camera_status,uint8_t error_code,uint8_t usb_speed_mode,const char *camera_name,const char *camera_type,const char *connection_port,const char *serial_number,const char *codec,const char *encoder_mode,const char *rtsp_url_qgc,const char *rtsp_url_controller,const char *rtsp_url_laptop)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN];
    _mav_put_uint16_t(buf, 0, video_width);
    _mav_put_uint16_t(buf, 2, video_height);
    _mav_put_uint16_t(buf, 4, depth_width);
    _mav_put_uint16_t(buf, 6, depth_height);
    _mav_put_uint16_t(buf, 8, rotation);
    _mav_put_uint16_t(buf, 10, bitrate_kbps);
    _mav_put_uint16_t(buf, 12, bitrate_max_kbps);
    _mav_put_uint16_t(buf, 14, vbv_buffer_kb);
    _mav_put_uint16_t(buf, 16, rtsp_port);
    _mav_put_uint8_t(buf, 18, video_fps);
    _mav_put_uint8_t(buf, 19, depth_fps);
    _mav_put_uint8_t(buf, 20, camera_id);
    _mav_put_uint8_t(buf, 21, is_default);
    _mav_put_uint8_t(buf, 22, profile_mode);
    _mav_put_uint8_t(buf, 23, enable_emitter);
    _mav_put_uint8_t(buf, 24, camera_status);
    _mav_put_uint8_t(buf, 25, error_code);
    _mav_put_uint8_t(buf, 26, usb_speed_mode);
    _mav_put_char_array(buf, 27, camera_name, 24);
    _mav_put_char_array(buf, 51, camera_type, 12);
    _mav_put_char_array(buf, 63, connection_port, 20);
    _mav_put_char_array(buf, 83, serial_number, 16);
    _mav_put_char_array(buf, 99, codec, 6);
    _mav_put_char_array(buf, 105, encoder_mode, 6);
    _mav_put_char_array(buf, 111, rtsp_url_qgc, 48);
    _mav_put_char_array(buf, 159, rtsp_url_controller, 48);
    _mav_put_char_array(buf, 207, rtsp_url_laptop, 40);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#else
    mavlink_cc_telemetry_camera_t packet;
    packet.video_width = video_width;
    packet.video_height = video_height;
    packet.depth_width = depth_width;
    packet.depth_height = depth_height;
    packet.rotation = rotation;
    packet.bitrate_kbps = bitrate_kbps;
    packet.bitrate_max_kbps = bitrate_max_kbps;
    packet.vbv_buffer_kb = vbv_buffer_kb;
    packet.rtsp_port = rtsp_port;
    packet.video_fps = video_fps;
    packet.depth_fps = depth_fps;
    packet.camera_id = camera_id;
    packet.is_default = is_default;
    packet.profile_mode = profile_mode;
    packet.enable_emitter = enable_emitter;
    packet.camera_status = camera_status;
    packet.error_code = error_code;
    packet.usb_speed_mode = usb_speed_mode;
    mav_array_memcpy(packet.camera_name, camera_name, sizeof(char)*24);
    mav_array_memcpy(packet.camera_type, camera_type, sizeof(char)*12);
    mav_array_memcpy(packet.connection_port, connection_port, sizeof(char)*20);
    mav_array_memcpy(packet.serial_number, serial_number, sizeof(char)*16);
    mav_array_memcpy(packet.codec, codec, sizeof(char)*6);
    mav_array_memcpy(packet.encoder_mode, encoder_mode, sizeof(char)*6);
    mav_array_memcpy(packet.rtsp_url_qgc, rtsp_url_qgc, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_controller, rtsp_url_controller, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_laptop, rtsp_url_laptop, sizeof(char)*40);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
}

/**
 * @brief Encode a cc_telemetry_camera struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_camera C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_telemetry_camera_t* cc_telemetry_camera)
{
    return mavlink_msg_cc_telemetry_camera_pack(system_id, component_id, msg, cc_telemetry_camera->video_width, cc_telemetry_camera->video_height, cc_telemetry_camera->depth_width, cc_telemetry_camera->depth_height, cc_telemetry_camera->rotation, cc_telemetry_camera->bitrate_kbps, cc_telemetry_camera->bitrate_max_kbps, cc_telemetry_camera->vbv_buffer_kb, cc_telemetry_camera->rtsp_port, cc_telemetry_camera->video_fps, cc_telemetry_camera->depth_fps, cc_telemetry_camera->camera_id, cc_telemetry_camera->is_default, cc_telemetry_camera->profile_mode, cc_telemetry_camera->enable_emitter, cc_telemetry_camera->camera_status, cc_telemetry_camera->error_code, cc_telemetry_camera->usb_speed_mode, cc_telemetry_camera->camera_name, cc_telemetry_camera->camera_type, cc_telemetry_camera->connection_port, cc_telemetry_camera->serial_number, cc_telemetry_camera->codec, cc_telemetry_camera->encoder_mode, cc_telemetry_camera->rtsp_url_qgc, cc_telemetry_camera->rtsp_url_controller, cc_telemetry_camera->rtsp_url_laptop);
}

/**
 * @brief Encode a cc_telemetry_camera struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_camera C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_telemetry_camera_t* cc_telemetry_camera)
{
    return mavlink_msg_cc_telemetry_camera_pack_chan(system_id, component_id, chan, msg, cc_telemetry_camera->video_width, cc_telemetry_camera->video_height, cc_telemetry_camera->depth_width, cc_telemetry_camera->depth_height, cc_telemetry_camera->rotation, cc_telemetry_camera->bitrate_kbps, cc_telemetry_camera->bitrate_max_kbps, cc_telemetry_camera->vbv_buffer_kb, cc_telemetry_camera->rtsp_port, cc_telemetry_camera->video_fps, cc_telemetry_camera->depth_fps, cc_telemetry_camera->camera_id, cc_telemetry_camera->is_default, cc_telemetry_camera->profile_mode, cc_telemetry_camera->enable_emitter, cc_telemetry_camera->camera_status, cc_telemetry_camera->error_code, cc_telemetry_camera->usb_speed_mode, cc_telemetry_camera->camera_name, cc_telemetry_camera->camera_type, cc_telemetry_camera->connection_port, cc_telemetry_camera->serial_number, cc_telemetry_camera->codec, cc_telemetry_camera->encoder_mode, cc_telemetry_camera->rtsp_url_qgc, cc_telemetry_camera->rtsp_url_controller, cc_telemetry_camera->rtsp_url_laptop);
}

/**
 * @brief Encode a cc_telemetry_camera struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_camera C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_telemetry_camera_t* cc_telemetry_camera)
{
    return mavlink_msg_cc_telemetry_camera_pack_status(system_id, component_id, _status, msg,  cc_telemetry_camera->video_width, cc_telemetry_camera->video_height, cc_telemetry_camera->depth_width, cc_telemetry_camera->depth_height, cc_telemetry_camera->rotation, cc_telemetry_camera->bitrate_kbps, cc_telemetry_camera->bitrate_max_kbps, cc_telemetry_camera->vbv_buffer_kb, cc_telemetry_camera->rtsp_port, cc_telemetry_camera->video_fps, cc_telemetry_camera->depth_fps, cc_telemetry_camera->camera_id, cc_telemetry_camera->is_default, cc_telemetry_camera->profile_mode, cc_telemetry_camera->enable_emitter, cc_telemetry_camera->camera_status, cc_telemetry_camera->error_code, cc_telemetry_camera->usb_speed_mode, cc_telemetry_camera->camera_name, cc_telemetry_camera->camera_type, cc_telemetry_camera->connection_port, cc_telemetry_camera->serial_number, cc_telemetry_camera->codec, cc_telemetry_camera->encoder_mode, cc_telemetry_camera->rtsp_url_qgc, cc_telemetry_camera->rtsp_url_controller, cc_telemetry_camera->rtsp_url_laptop);
}

/**
 * @brief Send a cc_telemetry_camera message
 * @param chan MAVLink channel to send the message
 *
 * @param video_width  Video stream resolution width in pixels, e.g. 1280 or 1920
 * @param video_height  Video stream resolution height in pixels, e.g. 720 or 1080
 * @param depth_width  Depth stream resolution width in pixels, e.g. 640
 * @param depth_height  Depth stream resolution height in pixels, e.g. 480
 * @param rotation  Camera sensor rotation in degrees: 0, 90, 180, 270
 * @param bitrate_kbps  Encoder nominal target bitrate in kbps, e.g. 2500
 * @param bitrate_max_kbps  Encoder maximum peak bitrate in kbps, e.g. 3000
 * @param vbv_buffer_kb  Encoder VBV buffer size in kilobits, e.g. 1250
 * @param rtsp_port  RTSP streaming server network port, e.g. 8554
 * @param video_fps  Video stream framerate, e.g. 30
 * @param depth_fps  Depth sensor stream framerate, e.g. 30
 * @param camera_id  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV)
 * @param is_default  Configuration default persistence flag (1: saved as default, 0: pending/modified)
 * @param profile_mode  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud
 * @param enable_emitter  Laser projector emitter flag (0: off, 1: on)
 * @param camera_status  Active camera operational state
 * @param error_code  Specific diagnostic error code
 * @param usb_speed_mode  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2
 * @param camera_name  Human-readable camera model, e.g. Intel RealSense D435i
 * @param camera_type  Driver type: realsense, v4l2, siyi, csi
 * @param connection_port  Hardware bus or port identifier, e.g. USB 3.2 Gen 1
 * @param serial_number  Camera hardware serial number
 * @param codec  Video codec: h264, h265
 * @param encoder_mode  Encoder mode: hw, cpu, auto
 * @param rtsp_url_qgc  RTSP URL endpoint for QGroundControl (AP subnet)
 * @param rtsp_url_controller  RTSP URL endpoint for Handheld Controller / SIYI link
 * @param rtsp_url_laptop  RTSP URL endpoint for Laptop / LAN client
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_telemetry_camera_send(mavlink_channel_t chan, uint16_t video_width, uint16_t video_height, uint16_t depth_width, uint16_t depth_height, uint16_t rotation, uint16_t bitrate_kbps, uint16_t bitrate_max_kbps, uint16_t vbv_buffer_kb, uint16_t rtsp_port, uint8_t video_fps, uint8_t depth_fps, uint8_t camera_id, uint8_t is_default, uint8_t profile_mode, uint8_t enable_emitter, uint8_t camera_status, uint8_t error_code, uint8_t usb_speed_mode, const char *camera_name, const char *camera_type, const char *connection_port, const char *serial_number, const char *codec, const char *encoder_mode, const char *rtsp_url_qgc, const char *rtsp_url_controller, const char *rtsp_url_laptop)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN];
    _mav_put_uint16_t(buf, 0, video_width);
    _mav_put_uint16_t(buf, 2, video_height);
    _mav_put_uint16_t(buf, 4, depth_width);
    _mav_put_uint16_t(buf, 6, depth_height);
    _mav_put_uint16_t(buf, 8, rotation);
    _mav_put_uint16_t(buf, 10, bitrate_kbps);
    _mav_put_uint16_t(buf, 12, bitrate_max_kbps);
    _mav_put_uint16_t(buf, 14, vbv_buffer_kb);
    _mav_put_uint16_t(buf, 16, rtsp_port);
    _mav_put_uint8_t(buf, 18, video_fps);
    _mav_put_uint8_t(buf, 19, depth_fps);
    _mav_put_uint8_t(buf, 20, camera_id);
    _mav_put_uint8_t(buf, 21, is_default);
    _mav_put_uint8_t(buf, 22, profile_mode);
    _mav_put_uint8_t(buf, 23, enable_emitter);
    _mav_put_uint8_t(buf, 24, camera_status);
    _mav_put_uint8_t(buf, 25, error_code);
    _mav_put_uint8_t(buf, 26, usb_speed_mode);
    _mav_put_char_array(buf, 27, camera_name, 24);
    _mav_put_char_array(buf, 51, camera_type, 12);
    _mav_put_char_array(buf, 63, connection_port, 20);
    _mav_put_char_array(buf, 83, serial_number, 16);
    _mav_put_char_array(buf, 99, codec, 6);
    _mav_put_char_array(buf, 105, encoder_mode, 6);
    _mav_put_char_array(buf, 111, rtsp_url_qgc, 48);
    _mav_put_char_array(buf, 159, rtsp_url_controller, 48);
    _mav_put_char_array(buf, 207, rtsp_url_laptop, 40);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, buf, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
#else
    mavlink_cc_telemetry_camera_t packet;
    packet.video_width = video_width;
    packet.video_height = video_height;
    packet.depth_width = depth_width;
    packet.depth_height = depth_height;
    packet.rotation = rotation;
    packet.bitrate_kbps = bitrate_kbps;
    packet.bitrate_max_kbps = bitrate_max_kbps;
    packet.vbv_buffer_kb = vbv_buffer_kb;
    packet.rtsp_port = rtsp_port;
    packet.video_fps = video_fps;
    packet.depth_fps = depth_fps;
    packet.camera_id = camera_id;
    packet.is_default = is_default;
    packet.profile_mode = profile_mode;
    packet.enable_emitter = enable_emitter;
    packet.camera_status = camera_status;
    packet.error_code = error_code;
    packet.usb_speed_mode = usb_speed_mode;
    mav_array_memcpy(packet.camera_name, camera_name, sizeof(char)*24);
    mav_array_memcpy(packet.camera_type, camera_type, sizeof(char)*12);
    mav_array_memcpy(packet.connection_port, connection_port, sizeof(char)*20);
    mav_array_memcpy(packet.serial_number, serial_number, sizeof(char)*16);
    mav_array_memcpy(packet.codec, codec, sizeof(char)*6);
    mav_array_memcpy(packet.encoder_mode, encoder_mode, sizeof(char)*6);
    mav_array_memcpy(packet.rtsp_url_qgc, rtsp_url_qgc, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_controller, rtsp_url_controller, sizeof(char)*48);
    mav_array_memcpy(packet.rtsp_url_laptop, rtsp_url_laptop, sizeof(char)*40);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, (const char *)&packet, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
#endif
}

/**
 * @brief Send a cc_telemetry_camera message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_telemetry_camera_send_struct(mavlink_channel_t chan, const mavlink_cc_telemetry_camera_t* cc_telemetry_camera)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_telemetry_camera_send(chan, cc_telemetry_camera->video_width, cc_telemetry_camera->video_height, cc_telemetry_camera->depth_width, cc_telemetry_camera->depth_height, cc_telemetry_camera->rotation, cc_telemetry_camera->bitrate_kbps, cc_telemetry_camera->bitrate_max_kbps, cc_telemetry_camera->vbv_buffer_kb, cc_telemetry_camera->rtsp_port, cc_telemetry_camera->video_fps, cc_telemetry_camera->depth_fps, cc_telemetry_camera->camera_id, cc_telemetry_camera->is_default, cc_telemetry_camera->profile_mode, cc_telemetry_camera->enable_emitter, cc_telemetry_camera->camera_status, cc_telemetry_camera->error_code, cc_telemetry_camera->usb_speed_mode, cc_telemetry_camera->camera_name, cc_telemetry_camera->camera_type, cc_telemetry_camera->connection_port, cc_telemetry_camera->serial_number, cc_telemetry_camera->codec, cc_telemetry_camera->encoder_mode, cc_telemetry_camera->rtsp_url_qgc, cc_telemetry_camera->rtsp_url_controller, cc_telemetry_camera->rtsp_url_laptop);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, (const char *)cc_telemetry_camera, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_telemetry_camera_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint16_t video_width, uint16_t video_height, uint16_t depth_width, uint16_t depth_height, uint16_t rotation, uint16_t bitrate_kbps, uint16_t bitrate_max_kbps, uint16_t vbv_buffer_kb, uint16_t rtsp_port, uint8_t video_fps, uint8_t depth_fps, uint8_t camera_id, uint8_t is_default, uint8_t profile_mode, uint8_t enable_emitter, uint8_t camera_status, uint8_t error_code, uint8_t usb_speed_mode, const char *camera_name, const char *camera_type, const char *connection_port, const char *serial_number, const char *codec, const char *encoder_mode, const char *rtsp_url_qgc, const char *rtsp_url_controller, const char *rtsp_url_laptop)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint16_t(buf, 0, video_width);
    _mav_put_uint16_t(buf, 2, video_height);
    _mav_put_uint16_t(buf, 4, depth_width);
    _mav_put_uint16_t(buf, 6, depth_height);
    _mav_put_uint16_t(buf, 8, rotation);
    _mav_put_uint16_t(buf, 10, bitrate_kbps);
    _mav_put_uint16_t(buf, 12, bitrate_max_kbps);
    _mav_put_uint16_t(buf, 14, vbv_buffer_kb);
    _mav_put_uint16_t(buf, 16, rtsp_port);
    _mav_put_uint8_t(buf, 18, video_fps);
    _mav_put_uint8_t(buf, 19, depth_fps);
    _mav_put_uint8_t(buf, 20, camera_id);
    _mav_put_uint8_t(buf, 21, is_default);
    _mav_put_uint8_t(buf, 22, profile_mode);
    _mav_put_uint8_t(buf, 23, enable_emitter);
    _mav_put_uint8_t(buf, 24, camera_status);
    _mav_put_uint8_t(buf, 25, error_code);
    _mav_put_uint8_t(buf, 26, usb_speed_mode);
    _mav_put_char_array(buf, 27, camera_name, 24);
    _mav_put_char_array(buf, 51, camera_type, 12);
    _mav_put_char_array(buf, 63, connection_port, 20);
    _mav_put_char_array(buf, 83, serial_number, 16);
    _mav_put_char_array(buf, 99, codec, 6);
    _mav_put_char_array(buf, 105, encoder_mode, 6);
    _mav_put_char_array(buf, 111, rtsp_url_qgc, 48);
    _mav_put_char_array(buf, 159, rtsp_url_controller, 48);
    _mav_put_char_array(buf, 207, rtsp_url_laptop, 40);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, buf, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
#else
    mavlink_cc_telemetry_camera_t *packet = (mavlink_cc_telemetry_camera_t *)msgbuf;
    packet->video_width = video_width;
    packet->video_height = video_height;
    packet->depth_width = depth_width;
    packet->depth_height = depth_height;
    packet->rotation = rotation;
    packet->bitrate_kbps = bitrate_kbps;
    packet->bitrate_max_kbps = bitrate_max_kbps;
    packet->vbv_buffer_kb = vbv_buffer_kb;
    packet->rtsp_port = rtsp_port;
    packet->video_fps = video_fps;
    packet->depth_fps = depth_fps;
    packet->camera_id = camera_id;
    packet->is_default = is_default;
    packet->profile_mode = profile_mode;
    packet->enable_emitter = enable_emitter;
    packet->camera_status = camera_status;
    packet->error_code = error_code;
    packet->usb_speed_mode = usb_speed_mode;
    mav_array_memcpy(packet->camera_name, camera_name, sizeof(char)*24);
    mav_array_memcpy(packet->camera_type, camera_type, sizeof(char)*12);
    mav_array_memcpy(packet->connection_port, connection_port, sizeof(char)*20);
    mav_array_memcpy(packet->serial_number, serial_number, sizeof(char)*16);
    mav_array_memcpy(packet->codec, codec, sizeof(char)*6);
    mav_array_memcpy(packet->encoder_mode, encoder_mode, sizeof(char)*6);
    mav_array_memcpy(packet->rtsp_url_qgc, rtsp_url_qgc, sizeof(char)*48);
    mav_array_memcpy(packet->rtsp_url_controller, rtsp_url_controller, sizeof(char)*48);
    mav_array_memcpy(packet->rtsp_url_laptop, rtsp_url_laptop, sizeof(char)*40);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, (const char *)packet, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_TELEMETRY_CAMERA UNPACKING


/**
 * @brief Get field video_width from cc_telemetry_camera message
 *
 * @return  Video stream resolution width in pixels, e.g. 1280 or 1920
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_video_width(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Get field video_height from cc_telemetry_camera message
 *
 * @return  Video stream resolution height in pixels, e.g. 720 or 1080
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_video_height(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  2);
}

/**
 * @brief Get field depth_width from cc_telemetry_camera message
 *
 * @return  Depth stream resolution width in pixels, e.g. 640
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_depth_width(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  4);
}

/**
 * @brief Get field depth_height from cc_telemetry_camera message
 *
 * @return  Depth stream resolution height in pixels, e.g. 480
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_depth_height(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  6);
}

/**
 * @brief Get field rotation from cc_telemetry_camera message
 *
 * @return  Camera sensor rotation in degrees: 0, 90, 180, 270
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_rotation(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  8);
}

/**
 * @brief Get field bitrate_kbps from cc_telemetry_camera message
 *
 * @return  Encoder nominal target bitrate in kbps, e.g. 2500
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_bitrate_kbps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  10);
}

/**
 * @brief Get field bitrate_max_kbps from cc_telemetry_camera message
 *
 * @return  Encoder maximum peak bitrate in kbps, e.g. 3000
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_bitrate_max_kbps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  12);
}

/**
 * @brief Get field vbv_buffer_kb from cc_telemetry_camera message
 *
 * @return  Encoder VBV buffer size in kilobits, e.g. 1250
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_vbv_buffer_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  14);
}

/**
 * @brief Get field rtsp_port from cc_telemetry_camera message
 *
 * @return  RTSP streaming server network port, e.g. 8554
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_rtsp_port(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  16);
}

/**
 * @brief Get field video_fps from cc_telemetry_camera message
 *
 * @return  Video stream framerate, e.g. 30
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_video_fps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  18);
}

/**
 * @brief Get field depth_fps from cc_telemetry_camera message
 *
 * @return  Depth sensor stream framerate, e.g. 30
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_depth_fps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  19);
}

/**
 * @brief Get field camera_id from cc_telemetry_camera message
 *
 * @return  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV)
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_camera_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  20);
}

/**
 * @brief Get field is_default from cc_telemetry_camera message
 *
 * @return  Configuration default persistence flag (1: saved as default, 0: pending/modified)
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_is_default(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  21);
}

/**
 * @brief Get field profile_mode from cc_telemetry_camera message
 *
 * @return  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_profile_mode(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  22);
}

/**
 * @brief Get field enable_emitter from cc_telemetry_camera message
 *
 * @return  Laser projector emitter flag (0: off, 1: on)
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_enable_emitter(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  23);
}

/**
 * @brief Get field camera_status from cc_telemetry_camera message
 *
 * @return  Active camera operational state
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_camera_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  24);
}

/**
 * @brief Get field error_code from cc_telemetry_camera message
 *
 * @return  Specific diagnostic error code
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_error_code(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  25);
}

/**
 * @brief Get field usb_speed_mode from cc_telemetry_camera message
 *
 * @return  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2
 */
static inline uint8_t mavlink_msg_cc_telemetry_camera_get_usb_speed_mode(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  26);
}

/**
 * @brief Get field camera_name from cc_telemetry_camera message
 *
 * @return  Human-readable camera model, e.g. Intel RealSense D435i
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_camera_name(const mavlink_message_t* msg, char *camera_name)
{
    return _MAV_RETURN_char_array(msg, camera_name, 24,  27);
}

/**
 * @brief Get field camera_type from cc_telemetry_camera message
 *
 * @return  Driver type: realsense, v4l2, siyi, csi
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_camera_type(const mavlink_message_t* msg, char *camera_type)
{
    return _MAV_RETURN_char_array(msg, camera_type, 12,  51);
}

/**
 * @brief Get field connection_port from cc_telemetry_camera message
 *
 * @return  Hardware bus or port identifier, e.g. USB 3.2 Gen 1
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_connection_port(const mavlink_message_t* msg, char *connection_port)
{
    return _MAV_RETURN_char_array(msg, connection_port, 20,  63);
}

/**
 * @brief Get field serial_number from cc_telemetry_camera message
 *
 * @return  Camera hardware serial number
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_serial_number(const mavlink_message_t* msg, char *serial_number)
{
    return _MAV_RETURN_char_array(msg, serial_number, 16,  83);
}

/**
 * @brief Get field codec from cc_telemetry_camera message
 *
 * @return  Video codec: h264, h265
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_codec(const mavlink_message_t* msg, char *codec)
{
    return _MAV_RETURN_char_array(msg, codec, 6,  99);
}

/**
 * @brief Get field encoder_mode from cc_telemetry_camera message
 *
 * @return  Encoder mode: hw, cpu, auto
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_encoder_mode(const mavlink_message_t* msg, char *encoder_mode)
{
    return _MAV_RETURN_char_array(msg, encoder_mode, 6,  105);
}

/**
 * @brief Get field rtsp_url_qgc from cc_telemetry_camera message
 *
 * @return  RTSP URL endpoint for QGroundControl (AP subnet)
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_rtsp_url_qgc(const mavlink_message_t* msg, char *rtsp_url_qgc)
{
    return _MAV_RETURN_char_array(msg, rtsp_url_qgc, 48,  111);
}

/**
 * @brief Get field rtsp_url_controller from cc_telemetry_camera message
 *
 * @return  RTSP URL endpoint for Handheld Controller / SIYI link
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_rtsp_url_controller(const mavlink_message_t* msg, char *rtsp_url_controller)
{
    return _MAV_RETURN_char_array(msg, rtsp_url_controller, 48,  159);
}

/**
 * @brief Get field rtsp_url_laptop from cc_telemetry_camera message
 *
 * @return  RTSP URL endpoint for Laptop / LAN client
 */
static inline uint16_t mavlink_msg_cc_telemetry_camera_get_rtsp_url_laptop(const mavlink_message_t* msg, char *rtsp_url_laptop)
{
    return _MAV_RETURN_char_array(msg, rtsp_url_laptop, 40,  207);
}

/**
 * @brief Decode a cc_telemetry_camera message into a struct
 *
 * @param msg The message to decode
 * @param cc_telemetry_camera C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_telemetry_camera_decode(const mavlink_message_t* msg, mavlink_cc_telemetry_camera_t* cc_telemetry_camera)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_telemetry_camera->video_width = mavlink_msg_cc_telemetry_camera_get_video_width(msg);
    cc_telemetry_camera->video_height = mavlink_msg_cc_telemetry_camera_get_video_height(msg);
    cc_telemetry_camera->depth_width = mavlink_msg_cc_telemetry_camera_get_depth_width(msg);
    cc_telemetry_camera->depth_height = mavlink_msg_cc_telemetry_camera_get_depth_height(msg);
    cc_telemetry_camera->rotation = mavlink_msg_cc_telemetry_camera_get_rotation(msg);
    cc_telemetry_camera->bitrate_kbps = mavlink_msg_cc_telemetry_camera_get_bitrate_kbps(msg);
    cc_telemetry_camera->bitrate_max_kbps = mavlink_msg_cc_telemetry_camera_get_bitrate_max_kbps(msg);
    cc_telemetry_camera->vbv_buffer_kb = mavlink_msg_cc_telemetry_camera_get_vbv_buffer_kb(msg);
    cc_telemetry_camera->rtsp_port = mavlink_msg_cc_telemetry_camera_get_rtsp_port(msg);
    cc_telemetry_camera->video_fps = mavlink_msg_cc_telemetry_camera_get_video_fps(msg);
    cc_telemetry_camera->depth_fps = mavlink_msg_cc_telemetry_camera_get_depth_fps(msg);
    cc_telemetry_camera->camera_id = mavlink_msg_cc_telemetry_camera_get_camera_id(msg);
    cc_telemetry_camera->is_default = mavlink_msg_cc_telemetry_camera_get_is_default(msg);
    cc_telemetry_camera->profile_mode = mavlink_msg_cc_telemetry_camera_get_profile_mode(msg);
    cc_telemetry_camera->enable_emitter = mavlink_msg_cc_telemetry_camera_get_enable_emitter(msg);
    cc_telemetry_camera->camera_status = mavlink_msg_cc_telemetry_camera_get_camera_status(msg);
    cc_telemetry_camera->error_code = mavlink_msg_cc_telemetry_camera_get_error_code(msg);
    cc_telemetry_camera->usb_speed_mode = mavlink_msg_cc_telemetry_camera_get_usb_speed_mode(msg);
    mavlink_msg_cc_telemetry_camera_get_camera_name(msg, cc_telemetry_camera->camera_name);
    mavlink_msg_cc_telemetry_camera_get_camera_type(msg, cc_telemetry_camera->camera_type);
    mavlink_msg_cc_telemetry_camera_get_connection_port(msg, cc_telemetry_camera->connection_port);
    mavlink_msg_cc_telemetry_camera_get_serial_number(msg, cc_telemetry_camera->serial_number);
    mavlink_msg_cc_telemetry_camera_get_codec(msg, cc_telemetry_camera->codec);
    mavlink_msg_cc_telemetry_camera_get_encoder_mode(msg, cc_telemetry_camera->encoder_mode);
    mavlink_msg_cc_telemetry_camera_get_rtsp_url_qgc(msg, cc_telemetry_camera->rtsp_url_qgc);
    mavlink_msg_cc_telemetry_camera_get_rtsp_url_controller(msg, cc_telemetry_camera->rtsp_url_controller);
    mavlink_msg_cc_telemetry_camera_get_rtsp_url_laptop(msg, cc_telemetry_camera->rtsp_url_laptop);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN? msg->len : MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN;
        memset(cc_telemetry_camera, 0, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_LEN);
    memcpy(cc_telemetry_camera, _MAV_PAYLOAD(msg), len);
#endif
}
