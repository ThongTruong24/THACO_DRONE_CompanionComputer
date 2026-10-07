// MESSAGE CC_TELEMETRY_CAMERA support class

#pragma once

namespace mavlink {
namespace thaco {
namespace msg {

/**
 * @brief CC_TELEMETRY_CAMERA message
 *
 * Companion Computer camera sensor, multi-platform RTSP endpoints (QGC, Controller, Laptop), resolution, FPS, and default profile flag.
 */
struct CC_TELEMETRY_CAMERA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42011;
    static constexpr size_t LENGTH = 247;
    static constexpr size_t MIN_LENGTH = 247;
    static constexpr uint8_t CRC_EXTRA = 169;
    static constexpr auto NAME = "CC_TELEMETRY_CAMERA";


    uint16_t video_width; /*<  Video stream resolution width in pixels, e.g. 1280 or 1920 */
    uint16_t video_height; /*<  Video stream resolution height in pixels, e.g. 720 or 1080 */
    uint16_t depth_width; /*<  Depth stream resolution width in pixels, e.g. 640 */
    uint16_t depth_height; /*<  Depth stream resolution height in pixels, e.g. 480 */
    uint16_t rotation; /*<  Camera sensor rotation in degrees: 0, 90, 180, 270 */
    uint16_t bitrate_kbps; /*<  Encoder nominal target bitrate in kbps, e.g. 2500 */
    uint16_t bitrate_max_kbps; /*<  Encoder maximum peak bitrate in kbps, e.g. 3000 */
    uint16_t vbv_buffer_kb; /*<  Encoder VBV buffer size in kilobits, e.g. 1250 */
    uint16_t rtsp_port; /*<  RTSP streaming server network port, e.g. 8554 */
    uint8_t video_fps; /*<  Video stream framerate, e.g. 30 */
    uint8_t depth_fps; /*<  Depth sensor stream framerate, e.g. 30 */
    uint8_t camera_id; /*<  Camera index (0: Primary / RealSense, 1: Secondary / Gimbal or FPV) */
    uint8_t is_default; /*<  Configuration default persistence flag (1: saved as default, 0: pending/modified) */
    uint8_t profile_mode; /*<  0: rgb_only, 1: rgb_depth, 2: rgb_depth_pointcloud */
    uint8_t enable_emitter; /*<  Laser projector emitter flag (0: off, 1: on) */
    uint8_t camera_status; /*<  Active camera operational state */
    uint8_t error_code; /*<  Specific diagnostic error code */
    uint8_t usb_speed_mode; /*<  USB bus speed mode: 1: USB 2.0, 2: USB 3.0 (SuperSpeed), 3: USB 3.2 */
    std::array<char, 24> camera_name; /*<  Human-readable camera model, e.g. Intel RealSense D435i */
    std::array<char, 12> camera_type; /*<  Driver type: realsense, v4l2, siyi, csi */
    std::array<char, 20> connection_port; /*<  Hardware bus or port identifier, e.g. USB 3.2 Gen 1 */
    std::array<char, 16> serial_number; /*<  Camera hardware serial number */
    std::array<char, 6> codec; /*<  Video codec: h264, h265 */
    std::array<char, 6> encoder_mode; /*<  Encoder mode: hw, cpu, auto */
    std::array<char, 48> rtsp_url_qgc; /*<  RTSP URL endpoint for QGroundControl (AP subnet) */
    std::array<char, 48> rtsp_url_controller; /*<  RTSP URL endpoint for Handheld Controller / SIYI link */
    std::array<char, 40> rtsp_url_laptop; /*<  RTSP URL endpoint for Laptop / LAN client */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  video_width: " << video_width << std::endl;
        ss << "  video_height: " << video_height << std::endl;
        ss << "  depth_width: " << depth_width << std::endl;
        ss << "  depth_height: " << depth_height << std::endl;
        ss << "  rotation: " << rotation << std::endl;
        ss << "  bitrate_kbps: " << bitrate_kbps << std::endl;
        ss << "  bitrate_max_kbps: " << bitrate_max_kbps << std::endl;
        ss << "  vbv_buffer_kb: " << vbv_buffer_kb << std::endl;
        ss << "  rtsp_port: " << rtsp_port << std::endl;
        ss << "  video_fps: " << +video_fps << std::endl;
        ss << "  depth_fps: " << +depth_fps << std::endl;
        ss << "  camera_id: " << +camera_id << std::endl;
        ss << "  is_default: " << +is_default << std::endl;
        ss << "  profile_mode: " << +profile_mode << std::endl;
        ss << "  enable_emitter: " << +enable_emitter << std::endl;
        ss << "  camera_status: " << +camera_status << std::endl;
        ss << "  error_code: " << +error_code << std::endl;
        ss << "  usb_speed_mode: " << +usb_speed_mode << std::endl;
        ss << "  camera_name: \"" << to_string(camera_name) << "\"" << std::endl;
        ss << "  camera_type: \"" << to_string(camera_type) << "\"" << std::endl;
        ss << "  connection_port: \"" << to_string(connection_port) << "\"" << std::endl;
        ss << "  serial_number: \"" << to_string(serial_number) << "\"" << std::endl;
        ss << "  codec: \"" << to_string(codec) << "\"" << std::endl;
        ss << "  encoder_mode: \"" << to_string(encoder_mode) << "\"" << std::endl;
        ss << "  rtsp_url_qgc: \"" << to_string(rtsp_url_qgc) << "\"" << std::endl;
        ss << "  rtsp_url_controller: \"" << to_string(rtsp_url_controller) << "\"" << std::endl;
        ss << "  rtsp_url_laptop: \"" << to_string(rtsp_url_laptop) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << video_width;                   // offset: 0
        map << video_height;                  // offset: 2
        map << depth_width;                   // offset: 4
        map << depth_height;                  // offset: 6
        map << rotation;                      // offset: 8
        map << bitrate_kbps;                  // offset: 10
        map << bitrate_max_kbps;              // offset: 12
        map << vbv_buffer_kb;                 // offset: 14
        map << rtsp_port;                     // offset: 16
        map << video_fps;                     // offset: 18
        map << depth_fps;                     // offset: 19
        map << camera_id;                     // offset: 20
        map << is_default;                    // offset: 21
        map << profile_mode;                  // offset: 22
        map << enable_emitter;                // offset: 23
        map << camera_status;                 // offset: 24
        map << error_code;                    // offset: 25
        map << usb_speed_mode;                // offset: 26
        map << camera_name;                   // offset: 27
        map << camera_type;                   // offset: 51
        map << connection_port;               // offset: 63
        map << serial_number;                 // offset: 83
        map << codec;                         // offset: 99
        map << encoder_mode;                  // offset: 105
        map << rtsp_url_qgc;                  // offset: 111
        map << rtsp_url_controller;           // offset: 159
        map << rtsp_url_laptop;               // offset: 207
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> video_width;                   // offset: 0
        map >> video_height;                  // offset: 2
        map >> depth_width;                   // offset: 4
        map >> depth_height;                  // offset: 6
        map >> rotation;                      // offset: 8
        map >> bitrate_kbps;                  // offset: 10
        map >> bitrate_max_kbps;              // offset: 12
        map >> vbv_buffer_kb;                 // offset: 14
        map >> rtsp_port;                     // offset: 16
        map >> video_fps;                     // offset: 18
        map >> depth_fps;                     // offset: 19
        map >> camera_id;                     // offset: 20
        map >> is_default;                    // offset: 21
        map >> profile_mode;                  // offset: 22
        map >> enable_emitter;                // offset: 23
        map >> camera_status;                 // offset: 24
        map >> error_code;                    // offset: 25
        map >> usb_speed_mode;                // offset: 26
        map >> camera_name;                   // offset: 27
        map >> camera_type;                   // offset: 51
        map >> connection_port;               // offset: 63
        map >> serial_number;                 // offset: 83
        map >> codec;                         // offset: 99
        map >> encoder_mode;                  // offset: 105
        map >> rtsp_url_qgc;                  // offset: 111
        map >> rtsp_url_controller;           // offset: 159
        map >> rtsp_url_laptop;               // offset: 207
    }
};

} // namespace msg
} // namespace thaco
} // namespace mavlink
