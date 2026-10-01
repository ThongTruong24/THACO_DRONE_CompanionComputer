#pragma once

#include "camera_interface.hpp"
#include <atomic>
#include <vector>

namespace drone {

class V4L2Camera : public ICameraDevice {
public:
    V4L2Camera();
    ~V4L2Camera() override;

    bool initialize(const CameraConfig& config) override;
    bool startStream() override;
    void stopStream() override;
    bool captureFrame(FrameData& out_frame) override;
    bool isHealthy() const override;
    bool isAvailable() const override;
    std::string getDeviceName() const override;
    StreamErrorCode getLastErrorCode() const override;

private:
    struct Buffer {
        void* start;
        size_t length;
    };

    CameraConfig m_config;
    int m_fd;
    std::vector<Buffer> m_buffers;
    std::atomic<bool> m_is_streaming;
    std::atomic<bool> m_is_healthy;
    std::atomic<StreamErrorCode> m_last_error;
    std::string m_device_name;
    uint64_t m_frame_counter;
};

} // namespace drone
