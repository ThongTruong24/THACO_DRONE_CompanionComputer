#include "config_manager.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>

namespace drone {

bool ConfigManager::loadConfig(const std::string& yaml_path, CameraConfig& out_config) {
    try {
        YAML::Node root = YAML::LoadFile(yaml_path);

        if (root["camera_type"]) {
            out_config.camera_type = root["camera_type"].as<std::string>();
        }

        if (root["device_settings"]) {
            auto dev = root["device_settings"];
            if (dev["realsense"]) {
                auto rs = dev["realsense"];
                if (rs["serial_number"]) out_config.realsense.serial_number = rs["serial_number"].as<std::string>();
                if (rs["profile_mode"]) out_config.realsense.profile_mode = rs["profile_mode"].as<std::string>();
                if (rs["enable_emitter"]) out_config.realsense.enable_emitter = rs["enable_emitter"].as<bool>();
                if (rs["depth_width"]) out_config.realsense.depth_width = rs["depth_width"].as<int>();
                if (rs["depth_height"]) out_config.realsense.depth_height = rs["depth_height"].as<int>();
                if (rs["depth_fps"]) out_config.realsense.depth_fps = rs["depth_fps"].as<int>();
            }
            if (dev["v4l2"]) {
                auto v = dev["v4l2"];
                if (v["device_path"]) out_config.v4l2.device_path = v["device_path"].as<std::string>();
                if (v["pixel_format"]) out_config.v4l2.pixel_format = v["pixel_format"].as<std::string>();
                if (v["num_buffers"]) out_config.v4l2.num_buffers = v["num_buffers"].as<int>();
            }
        }

        if (root["video"]) {
            auto v = root["video"];
            if (v["width"]) out_config.video.width = v["width"].as<int>();
            if (v["height"]) out_config.video.height = v["height"].as<int>();
            if (v["fps"]) out_config.video.fps = v["fps"].as<int>();
            if (v["rotation"]) out_config.video.rotation = v["rotation"].as<int>();
        }

        if (root["encoder"]) {
            auto enc = root["encoder"];
            if (enc["codec"]) out_config.encoder.codec = enc["codec"].as<std::string>();
            if (enc["mode"]) out_config.encoder.mode = enc["mode"].as<std::string>();
            if (enc["bitrate_kbps"]) out_config.encoder.bitrate_kbps = enc["bitrate_kbps"].as<int>();
            if (enc["bitrate_min_kbps"]) out_config.encoder.bitrate_min_kbps = enc["bitrate_min_kbps"].as<int>();
            if (enc["bitrate_max_kbps"]) out_config.encoder.bitrate_max_kbps = enc["bitrate_max_kbps"].as<int>();
            if (enc["vbv_buffer_kb"]) out_config.encoder.vbv_buffer_kb = enc["vbv_buffer_kb"].as<int>();
            if (enc["tune"]) out_config.encoder.tune = enc["tune"].as<std::string>();
            if (enc["speed_preset"]) out_config.encoder.speed_preset = enc["speed_preset"].as<std::string>();
            if (enc["threads"]) out_config.encoder.threads = enc["threads"].as<int>();
            if (enc["intra_refresh"]) out_config.encoder.intra_refresh = enc["intra_refresh"].as<bool>();
            if (enc["adaptive_rate"]) out_config.encoder.adaptive_rate = enc["adaptive_rate"].as<bool>();
            if (enc["key_int_max"]) out_config.encoder.key_int_max = enc["key_int_max"].as<int>();
            if (enc["leaky_queue_buffers"]) out_config.encoder.leaky_queue_buffers = enc["leaky_queue_buffers"].as<int>();
        }

        if (root["network"]) {
            auto net = root["network"];
            if (net["sink_type"]) out_config.network.sink_type = net["sink_type"].as<std::string>();
            if (net["target_url"]) out_config.network.target_url = net["target_url"].as<std::string>();
            if (net["reconnect_interval_ms"]) out_config.network.reconnect_interval_ms = net["reconnect_interval_ms"].as<int>();
            if (net["timeout_seconds"]) out_config.network.timeout_seconds = net["timeout_seconds"].as<int>();
        }

        if (out_config.encoder.mode != "auto" && out_config.encoder.mode != "hardware" &&
            out_config.encoder.mode != "software") {
            throw std::runtime_error("encoder.mode must be auto, hardware, or software");
        }
        if (out_config.video.width <= 0 || out_config.video.height <= 0 || out_config.video.fps <= 0 ||
            out_config.encoder.bitrate_kbps <= 0 || out_config.encoder.threads <= 0 ||
            out_config.encoder.leaky_queue_buffers <= 0) {
            throw std::runtime_error("video and encoder numeric settings must be positive");
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager][ERROR] Lỗi đọc file cấu hình " << yaml_path << ": " << e.what() << "\n";
        return false;
    }
}

void ConfigManager::printConfig(const CameraConfig& c) {
    std::cout << "==========================================================\n"
              << "       CẤU HÌNH CAMERA STREAMER (C++ SOLID ENGINE)       \n"
              << "==========================================================\n"
              << "  Camera Type    : " << c.camera_type << "\n"
              << "  Profile Mode   : " << c.realsense.profile_mode << "\n"
              << "  Resolution     : " << c.video.width << "x" << c.video.height << " @" << c.video.fps << "fps\n"
              << "  Rotation       : " << c.video.rotation << " deg (SIMD NEON)\n"
              << "  Encoder        : " << c.encoder.mode << " H.264 | Preset: " << c.encoder.speed_preset << " | Tune: " << c.encoder.tune << "\n"
              << "  Bitrate        : " << c.encoder.bitrate_kbps << " kbps (Min: " << c.encoder.bitrate_min_kbps << ", Max: " << c.encoder.bitrate_max_kbps << " kbps)\n"
              << "  Adaptive Rate  : " << (c.encoder.adaptive_rate ? "ACTIVE (AIMD)" : "OFF") << "\n"
              << "  Intra-Refresh  : " << (c.encoder.intra_refresh ? "ACTIVE (Anti-Spike)" : "OFF") << "\n"
              << "  Leaky Queue    : " << c.encoder.leaky_queue_buffers << " buffers (Anti-Overflow)\n"
              << "  Target URL     : " << c.network.target_url << "\n"
              << "==========================================================\n";
}

} // namespace drone
