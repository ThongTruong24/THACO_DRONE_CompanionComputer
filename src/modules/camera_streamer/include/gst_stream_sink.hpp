#pragma once

#include "ICameraDevice.hpp"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <atomic>
#include <string>
#include <chrono>
#include <mutex>

namespace drone
{

class GstStreamSink : public IGstStreamSink
{
public:
	GstStreamSink();
	~GstStreamSink() override;

	bool start(const CameraConfig &config) override;
	bool pushFrame(const FrameData &frame) override;
	void stop() override;
	bool isConnected() const override;
	StreamErrorCode getLastErrorCode() const override;

	int getCurrentBitrate() const override;
	uint64_t getDroppedFrames() const override;
	void forceKeyframe() override;
	void setBitrate(int bitrate_kbps) override;
	void setEnabled(bool enabled) override;
	bool isEnabled() const { return m_enabled.load(); }

private:
	bool buildPipelineLocked();
	void stopLocked();
	static gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer user_data);
	static void onQueueOverrun(GstElement *queue, gpointer user_data);

	void handleOverrun();
	void checkAdaptiveRateRecovery();

	CameraConfig m_config;
	GstElement *m_pipeline;
	GstAppSrc *m_appsrc;
	GstElement *m_queue;
	GstElement *m_encoder;
	GstBus *m_bus;
	guint m_bus_watch_id;

	std::atomic<bool> m_enabled{true};
	std::atomic<bool> m_is_connected{false};
	std::atomic<StreamErrorCode> m_last_error{StreamErrorCode::SUCCESS};
	std::atomic<uint64_t> m_pushed_frames{0};
	uint64_t m_pts_duration{0};

	std::atomic<int> m_current_bitrate{1400};
	std::atomic<uint64_t> m_dropped_frames{0};
	std::chrono::steady_clock::time_point m_last_overrun_time;
	std::chrono::steady_clock::time_point m_last_increase_time;
	std::mutex m_rate_mutex;
	mutable std::mutex m_pipe_mutex;
};

} // namespace drone
