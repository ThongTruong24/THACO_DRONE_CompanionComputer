#pragma once

#include "camera_interface.hpp"
#include <string>

namespace drone {

class ConfigManager {
public:
    static bool loadConfig(const std::string& yaml_path, CameraConfig& out_config);
    static void printConfig(const CameraConfig& config);
};

} // namespace drone
