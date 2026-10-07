// MESSAGE CC_AI_VISION_CONTROL support class

#pragma once

namespace mavlink {
namespace thaco {
namespace msg {

/**
 * @brief CC_AI_VISION_CONTROL message
 *
 * Controls Companion Computer AI vision features from the ground control station.
 */
struct CC_AI_VISION_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42015;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 211;
    static constexpr auto NAME = "CC_AI_VISION_CONTROL";


    uint8_t bounding_box; /*<  Bounding box display control: 0 disabled, 1 enabled */
    uint8_t tracking; /*<  AI target tracking control: 0 disabled, 1 enabled */
    uint8_t following; /*<  Target following control: 0 disabled, 1 enabled */


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
        ss << "  bounding_box: " << +bounding_box << std::endl;
        ss << "  tracking: " << +tracking << std::endl;
        ss << "  following: " << +following << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << bounding_box;                  // offset: 0
        map << tracking;                      // offset: 1
        map << following;                     // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> bounding_box;                  // offset: 0
        map >> tracking;                      // offset: 1
        map >> following;                     // offset: 2
    }
};

} // namespace msg
} // namespace thaco
} // namespace mavlink
