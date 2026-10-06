// MESSAGE CC_TELEMETRY_VISION support class

#pragma once

namespace mavlink {
namespace thaco {
namespace msg {

/**
 * @brief CC_TELEMETRY_VISION message
 *
 * Companion Computer AI perception and YOLO object detector status.
 */
struct CC_TELEMETRY_VISION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42013;
    static constexpr size_t LENGTH = 51;
    static constexpr size_t MIN_LENGTH = 51;
    static constexpr uint8_t CRC_EXTRA = 234;
    static constexpr auto NAME = "CC_TELEMETRY_VISION";


    float confidence_thresh; /*<  Detection confidence threshold, e.g. 0.35 */
    float inference_fps; /*<  Current inference frames per second, e.g. 5.0 */
    uint16_t input_width; /*<  Model input width in pixels, e.g. 640 */
    uint16_t input_height; /*<  Model input height in pixels, e.g. 360 */
    uint8_t video_fps; /*<  Camera input FPS, e.g. 20 */
    uint8_t detections_count; /*<  Current count of detected target objects */
    uint8_t status_flags; /*<  Vision pipeline status: bit 0: running, bit 1: depth enabled */
    std::array<char, 24> model_name; /*<  Active YOLO model filename, e.g. yolo26n.pt */
    std::array<char, 12> input_source; /*<  Input video source: rtsp, realsense, v4l2 */


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
        ss << "  confidence_thresh: " << confidence_thresh << std::endl;
        ss << "  inference_fps: " << inference_fps << std::endl;
        ss << "  input_width: " << input_width << std::endl;
        ss << "  input_height: " << input_height << std::endl;
        ss << "  video_fps: " << +video_fps << std::endl;
        ss << "  detections_count: " << +detections_count << std::endl;
        ss << "  status_flags: " << +status_flags << std::endl;
        ss << "  model_name: \"" << to_string(model_name) << "\"" << std::endl;
        ss << "  input_source: \"" << to_string(input_source) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << confidence_thresh;             // offset: 0
        map << inference_fps;                 // offset: 4
        map << input_width;                   // offset: 8
        map << input_height;                  // offset: 10
        map << video_fps;                     // offset: 12
        map << detections_count;              // offset: 13
        map << status_flags;                  // offset: 14
        map << model_name;                    // offset: 15
        map << input_source;                  // offset: 39
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> confidence_thresh;             // offset: 0
        map >> inference_fps;                 // offset: 4
        map >> input_width;                   // offset: 8
        map >> input_height;                  // offset: 10
        map >> video_fps;                     // offset: 12
        map >> detections_count;              // offset: 13
        map >> status_flags;                  // offset: 14
        map >> model_name;                    // offset: 15
        map >> input_source;                  // offset: 39
    }
};

} // namespace msg
} // namespace thaco
} // namespace mavlink
