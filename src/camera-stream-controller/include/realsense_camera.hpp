#pragma once

#include "camera_interface.hpp"
#include <atomic>
#include <memory>

#if defined(HAVE_REALSENSE) && (HAVE_REALSENSE == 1)
#include <librealsense2/rs.hpp>
#endif

namespace drone {

class RealSenseCamera : public ICameraDevice {
public:
    RealSenseCamera();
    ~RealSenseCamera() override;

    bool initialize(const CameraConfig& config) override;
    bool startStream() override;
    void stopStream() override;
    bool captureFrame(FrameData& out_frame) override;
    bool isHealthy() const override;
    bool isAvailable() const override;
    std::string getDeviceName() const override;
    StreamErrorCode getLastErrorCode() const override;

private:
    CameraConfig m_config;
#if defined(HAVE_REALSENSE) && (HAVE_REALSENSE == 1)
    std::unique_ptr<rs2::pipeline> m_pipeline;
    std::unique_ptr<rs2::config> m_rs_config;
#endif
    std::atomic<bool> m_is_streaming;
    std::atomic<bool> m_is_healthy;
    std::atomic<StreamErrorCode> m_last_error;
    std::string m_device_name;
    uint64_t m_frame_counter;
};

} // namespace drone
