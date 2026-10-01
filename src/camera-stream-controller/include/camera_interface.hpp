#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <chrono>

namespace drone {

// ==============================================================================
// BỘ MÃ LỖI CHUẨN ĐOÁN HỆ THỐNG STREAM VIDEO
// ==============================================================================
enum class StreamErrorCode {
    SUCCESS = 0,
    ERR_CONFIG_INVALID,
    ERR_DEVICE_NOT_FOUND,
    ERR_USB_IO_ERROR,               // Tương đương UVC kernel EPROTO -71
    ERR_DEVICE_OPEN_FAILED,
    ERR_CAPTURE_TIMEOUT,
    ERR_RING_BUFFER_OVERFLOW,
    ERR_GST_PIPELINE_INIT_FAILED,
    ERR_GST_PUSH_BUFFER_FAILED,
    ERR_MEDIAMTX_UNREACHABLE,
    ERR_ENCODER_STALL,
    ERR_UNKNOWN
};

inline const char* errorCodeToString(StreamErrorCode code) {
    switch (code) {
        case StreamErrorCode::SUCCESS: return "SUCCESS";
        case StreamErrorCode::ERR_CONFIG_INVALID: return "ERR_CONFIG_INVALID";
        case StreamErrorCode::ERR_DEVICE_NOT_FOUND: return "ERR_DEVICE_NOT_FOUND";
        case StreamErrorCode::ERR_USB_IO_ERROR: return "ERR_USB_IO_ERROR (Kernel -71)";
        case StreamErrorCode::ERR_DEVICE_OPEN_FAILED: return "ERR_DEVICE_OPEN_FAILED";
        case StreamErrorCode::ERR_CAPTURE_TIMEOUT: return "ERR_CAPTURE_TIMEOUT";
        case StreamErrorCode::ERR_RING_BUFFER_OVERFLOW: return "ERR_RING_BUFFER_OVERFLOW";
        case StreamErrorCode::ERR_GST_PIPELINE_INIT_FAILED: return "ERR_GST_PIPELINE_INIT_FAILED";
        case StreamErrorCode::ERR_GST_PUSH_BUFFER_FAILED: return "ERR_GST_PUSH_BUFFER_FAILED";
        case StreamErrorCode::ERR_MEDIAMTX_UNREACHABLE: return "ERR_MEDIAMTX_UNREACHABLE";
        case StreamErrorCode::ERR_ENCODER_STALL: return "ERR_ENCODER_STALL";
        default: return "ERR_UNKNOWN";
    }
}

// ==============================================================================
// CẤU TRÚC DỮ LIỆU FRAME (ZERO-COPY READY)
// ==============================================================================
enum class PixelFormat {
    BGR8,
    RGB8,
    YUYV,
    I420
};

struct FrameData {
    std::vector<uint8_t> data; // Hoặc con trỏ chia sẻ
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::BGR8;
    uint64_t timestamp_us = 0;
    uint64_t frame_number = 0;
};

// ==============================================================================
// CẤU TRÚC CẤU HÌNH CAMERA TỔNG THỂ (TỪ camera.yaml, KHÔNG HARDCODE)
// ==============================================================================
struct RealSenseSettings {
    std::string serial_number;
    std::string profile_mode; // "rgb_only", "rgb_depth", "rgb_depth_pointcloud"
    bool enable_emitter = false;
    int depth_width = 640;
    int depth_height = 480;
    int depth_fps = 30;
};

struct V4L2Settings {
    std::string device_path = "/dev/video0";
    std::string pixel_format = "YUYV";
    int num_buffers = 4;
};

struct CSISettings {
    int camera_index = 0;
    int sensor_mode = 0;
};

struct VideoConfig {
    int width = 1280;
    int height = 720;
    int fps = 30;
    int rotation = 180; // 0, 90, 180, 270
};

struct EncoderConfig {
    std::string codec = "h264";
    // auto selects V4L2 H.264 when available; hardware requires it; software uses x264.
    std::string mode = "auto";
    int bitrate_kbps = 1400;
    int bitrate_min_kbps = 700;
    int bitrate_max_kbps = 2000;
    int vbv_buffer_kb = 1250;
    std::string tune = "zerolatency";
    std::string speed_preset = "ultrafast";
    int threads = 4;
    bool intra_refresh = false;
    bool adaptive_rate = true;
    int key_int_max = 30;
    int leaky_queue_buffers = 2;
};

struct NetworkConfig {
    std::string sink_type = "rtmp";
    std::string target_url = "rtmp://127.0.0.1:1935/camera";
    int reconnect_interval_ms = 1000;
    int timeout_seconds = 5;
};

struct CameraConfig {
    std::string camera_type = "realsense"; // "realsense", "v4l2", "csi", "mock"
    RealSenseSettings realsense;
    V4L2Settings v4l2;
    CSISettings csi;
    VideoConfig video;
    EncoderConfig encoder;
    NetworkConfig network;
};

// ==============================================================================
// GIAO DIỆN TRỪU TƯỢNG HÓA CAMERA (SOLID - SRP, OCP, LSP, ISP, DIP)
// ==============================================================================
class ICameraDevice {
public:
    virtual ~ICameraDevice() = default;

    virtual bool initialize(const CameraConfig& config) = 0;
    virtual bool startStream() = 0;
    virtual void stopStream() = 0;
    virtual bool captureFrame(FrameData& out_frame) = 0;
    virtual bool isHealthy() const = 0;
    virtual std::string getDeviceName() const = 0;
    virtual StreamErrorCode getLastErrorCode() const = 0;
    virtual bool isAvailable() const { return true; }
    virtual int getCurrentBitrate() const { return 0; }
    virtual uint64_t getDroppedFrames() const { return 0; }
    virtual void forceKeyframe() {}
    virtual void setBitrate(int bitrate_kbps) { (void)bitrate_kbps; }
};

// Giao diện xử lý hình ảnh (Xoay lật SIMD NEON)
class IFrameProcessor {
public:
    virtual ~IFrameProcessor() = default;
    virtual void process(FrameData& frame) = 0;
    virtual void setRotation(int rotation) { (void)rotation; }
    virtual int getRotation() const { return 0; }
};

// Giao diện xuất bản video GStreamer Sink
class IGstStreamSink {
public:
    virtual ~IGstStreamSink() = default;
    virtual bool start(const CameraConfig& config) = 0;
    virtual bool pushFrame(const FrameData& frame) = 0;
    virtual void stop() = 0;
    virtual bool isConnected() const = 0;
    virtual StreamErrorCode getLastErrorCode() const = 0;
    virtual bool isAvailable() const { return true; }
    virtual int getCurrentBitrate() const { return 0; }
    virtual uint64_t getDroppedFrames() const { return 0; }
    virtual void forceKeyframe() {}
    virtual void setBitrate(int bitrate_kbps) { (void)bitrate_kbps; }
};

} // namespace drone
