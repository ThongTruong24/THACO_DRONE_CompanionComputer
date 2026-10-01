#pragma once

#include "camera_interface.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace drone {

class StandbyGenerator {
public:
    StandbyGenerator(int width = 1280, int height = 720, int fps = 30);
    ~StandbyGenerator() = default;

    void updateConfig(int width, int height, int fps);
    void generateFrame(FrameData& out_frame, const std::string& status_message = "WAITING FOR REALSENSE CAMERA DEVICE...");

private:
    void renderBackground(uint8_t* buffer);
    void drawRect(uint8_t* buffer, int x, int y, int w, int h, uint8_t b, uint8_t g, uint8_t r);
    void drawText(uint8_t* buffer, int x, int y, const std::string& text, int scale, uint8_t b, uint8_t g, uint8_t r);
    void drawChar(uint8_t* buffer, int x, int y, char c, int scale, uint8_t b, uint8_t g, uint8_t r);

    int m_width;
    int m_height;
    int m_fps;
    uint64_t m_frame_count;
};

} // namespace drone
