#pragma once

#include "camera_interface.hpp"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <atomic>
#include <string>
#include <chrono>
#include <mutex>

namespace drone {

class GstStreamSink : public IGstStreamSink {
public:
    GstStreamSink();
    ~GstStreamSink() override;

    bool start(const CameraConfig& config) override;
    bool pushFrame(const FrameData& frame) override;
    void stop() override;
    bool isConnected() const override;
    StreamErrorCode getLastErrorCode() const override;

    int getCurrentBitrate() const override;
    uint64_t getDroppedFrames() const override;
    void forceKeyframe() override;
    void setBitrate(int bitrate_kbps) override;

private:
    bool buildPipeline();
    static gboolean busCallback(GstBus* bus, GstMessage* msg, gpointer user_data);
    static void onQueueOverrun(GstElement* queue, gpointer user_data);

    void handleOverrun();
    void checkAdaptiveRateRecovery();

    CameraConfig m_config;
    GstElement* m_pipeline;
    GstAppSrc* m_appsrc;
    GstElement* m_queue;
    GstElement* m_encoder;
    GstBus* m_bus;
    guint m_bus_watch_id;

    std::atomic<bool> m_is_connected;
    std::atomic<StreamErrorCode> m_last_error;
    std::atomic<uint64_t> m_pushed_frames;
    uint64_t m_pts_duration;

    std::atomic<int> m_current_bitrate;
    std::atomic<uint64_t> m_dropped_frames;
    std::chrono::steady_clock::time_point m_last_overrun_time;
    std::chrono::steady_clock::time_point m_last_increase_time;
    std::mutex m_rate_mutex;
};

} // namespace drone
