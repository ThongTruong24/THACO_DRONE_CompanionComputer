#include "gst_stream_sink.hpp"
#include <gst/video/video-event.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

namespace drone
{

GstStreamSink::GstStreamSink()
	: m_pipeline(nullptr),
	  m_appsrc(nullptr),
	  m_queue(nullptr),
	  m_encoder(nullptr),
	  m_bus(nullptr),
	  m_bus_watch_id(0),
	  m_enabled(true),
	  m_is_connected(false),
	  m_last_error(StreamErrorCode::SUCCESS),
	  m_pushed_frames(0),
	  m_pts_duration(0),
	  m_current_bitrate(1400),
	  m_dropped_frames(0)
{
	if (!gst_is_initialized()) {
		gst_init(nullptr, nullptr);
	}

	m_last_overrun_time = std::chrono::steady_clock::now();
	m_last_increase_time = std::chrono::steady_clock::now();
}

GstStreamSink::~GstStreamSink()
{
	stop();
}

bool GstStreamSink::start(const CameraConfig &config)
{
	std::lock_guard<std::mutex> lock(m_pipe_mutex);
	m_config = config;

	if (!m_enabled.load()) {
		return true;
	}

	return buildPipelineLocked();
}

void GstStreamSink::setEnabled(bool enabled)
{
	std::lock_guard<std::mutex> lock(m_pipe_mutex);

	if (m_enabled.load() == enabled) {
		return;
	}

	m_enabled.store(enabled);

	if (!enabled) {
		if (m_pipeline) {
			gst_element_set_state(m_pipeline, GST_STATE_NULL);
		}

		m_is_connected = false;
		std::cout << "[GstStreamSink] FPV disabled -> sink set to GST_STATE_NULL\n";

	} else {
		std::cout << "[GstStreamSink] FPV enabled -> restoring pipeline\n";

		if (m_pipeline) {
			GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

			if (ret != GST_STATE_CHANGE_FAILURE) {
				m_is_connected = true;

			} else {
				buildPipelineLocked();
			}

		} else {
			buildPipelineLocked();
		}
	}
}

bool GstStreamSink::buildPipelineLocked()
{
	stopLocked();

	int fps = m_config.video.fps > 0 ? m_config.video.fps : 30;
	m_pts_duration = GST_SECOND / fps;
	m_pushed_frames = 0;
	m_current_bitrate = m_config.encoder.bitrate_kbps;
	m_dropped_frames = 0;
	m_last_overrun_time = std::chrono::steady_clock::now();
	m_last_increase_time = std::chrono::steady_clock::now();

	GstElementFactory *hardware_factory = gst_element_factory_find("v4l2h264enc");
	const bool hardware_available = hardware_factory != nullptr;

	if (hardware_factory) {
		gst_object_unref(hardware_factory);
	}

	const bool use_hardware = m_config.encoder.mode != "software" && hardware_available;

	if (m_config.encoder.mode == "hardware" && !hardware_available) {
		std::cerr << "[GstStreamSink][WARN] Hardware H.264 was requested but v4l2h264enc is unavailable; "
			  << "falling back to x264 to preserve FPV.\n";
	}

	std::cout << "[GstStreamSink] Encoder selected: "
		  << (use_hardware ? "v4l2h264enc (hardware)" : "x264enc (software)") << "\n";

	std::ostringstream ss;
	ss << "appsrc name=mysource is-live=true do-timestamp=false format=time block=false "
	   << "max-buffers=" << m_config.encoder.leaky_queue_buffers << " leaky-type=downstream "
	   << "caps=video/x-raw,format=BGR,width=" << m_config.video.width << ",height=" << m_config.video.height
	   << ",framerate=" << fps << "/1 ! "
	   << "queue name=stream_queue max-size-buffers=" << m_config.encoder.leaky_queue_buffers
	   << " max-size-time=0 max-size-bytes=0 leaky=downstream ! "
	   << "videoconvert ! "
	   << "video/x-raw,format=I420 ! ";

	if (use_hardware) {
		ss << "v4l2h264enc name=stream_encoder extra-controls=controls,video_bitrate="
		   << (m_config.encoder.bitrate_kbps * 1000)
		   << ",h264_i_frame_period=" << m_config.encoder.key_int_max
		   << " ! video/x-h264,profile=baseline ! ";

	} else {
		ss << "x264enc name=stream_encoder tune=" << m_config.encoder.tune
		   << " speed-preset=" << m_config.encoder.speed_preset
		   << " bitrate=" << m_config.encoder.bitrate_kbps
		   << " pass=cbr vbv-buf-capacity=" << m_config.encoder.vbv_buffer_kb
		   << " threads=" << m_config.encoder.threads
		   << " intra-refresh=" << (m_config.encoder.intra_refresh ? "true" : "false")
		   << " key-int-max=" << m_config.encoder.key_int_max
		   << " bframes=0 byte-stream=false ! video/x-h264,profile=baseline ! ";
	}

	ss << "h264parse config-interval=-1 ! "
	   << "flvmux streamable=true ! "
	   << "rtmpsink location=" << m_config.network.target_url << " sync=false async=false";

	std::string pipe_str = ss.str();
	std::cout << "[GstStreamSink] Dựng pipeline GStreamer C++:\n" << pipe_str << "\n";

	GError *error = nullptr;
	m_pipeline = gst_parse_launch(pipe_str.c_str(), &error);

	if (!m_pipeline || error) {
		std::cerr << "[GstStreamSink][ERROR] gst_parse_launch thất bại: "
			  << (error ? error->message : "unknown") << "\n";

		if (error) { g_error_free(error); }

		if (use_hardware) {
			std::cerr << "[GstStreamSink][WARN] Hardware encoder pipeline could not be built; "
				  << "retrying with x264 to preserve FPV.\n";

			if (m_pipeline) {
				gst_object_unref(m_pipeline);
				m_pipeline = nullptr;
			}

			m_config.encoder.mode = "software";
			return buildPipelineLocked();
		}

		m_last_error = StreamErrorCode::ERR_GST_PIPELINE_INIT_FAILED;
		return false;
	}

	GstElement *src_elem = gst_bin_get_by_name(GST_BIN(m_pipeline), "mysource");

	if (!src_elem) {
		std::cerr << "[GstStreamSink][ERROR] Không tìm thấy phần tử appsrc 'mysource'\n";
		m_last_error = StreamErrorCode::ERR_GST_PIPELINE_INIT_FAILED;
		return false;
	}

	m_appsrc = GST_APP_SRC(src_elem);

	m_queue = gst_bin_get_by_name(GST_BIN(m_pipeline), "stream_queue");

	if (m_queue) {
		g_signal_connect(m_queue, "overrun", G_CALLBACK(onQueueOverrun), this);
	}

	m_encoder = gst_bin_get_by_name(GST_BIN(m_pipeline), "stream_encoder");

	m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	m_bus_watch_id = gst_bus_add_watch(m_bus, busCallback, this);

	GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_FAILURE) {
		std::cerr << "[GstStreamSink][ERROR] Không thể chuyển pipeline sang GST_STATE_PLAYING\n";

		if (use_hardware) {
			std::cerr << "[GstStreamSink][WARN] Hardware encoder could not start; retrying with x264 "
				  << "to preserve FPV.\n";
			m_config.encoder.mode = "software";
			return buildPipelineLocked();
		}

		m_last_error = StreamErrorCode::ERR_MEDIAMTX_UNREACHABLE;
		return false;
	}

	m_is_connected = true;
	m_last_error = StreamErrorCode::SUCCESS;
	std::cout << "[GstStreamSink] ✓ Pipeline GStreamer đang phát vào MediaMTX ("
		  << m_config.network.target_url << ")\n";
	return true;
}

void GstStreamSink::onQueueOverrun(GstElement * /*queue*/, gpointer user_data)
{
	auto *self = static_cast<GstStreamSink *>(user_data);

	if (self) {
		self->handleOverrun();
	}
}

void GstStreamSink::handleOverrun()
{
	m_dropped_frames++;
	auto now = std::chrono::steady_clock::now();
	m_last_overrun_time = now;

	static auto last_forced_idr = std::chrono::steady_clock::time_point{};

	if (std::chrono::duration<double>(now - last_forced_idr).count() >= 2.0) {
		last_forced_idr = now;
		forceKeyframe();
	}

	if (!m_config.encoder.adaptive_rate) {
		return;
	}

	std::lock_guard<std::mutex> lock(m_rate_mutex);
	int curr = m_current_bitrate.load();
	int next_br = std::max(m_config.encoder.bitrate_min_kbps, static_cast<int>(curr * 0.65));

	if (next_br < curr) {
		m_current_bitrate.store(next_br);

		if (m_encoder) {
			g_object_set(m_encoder, "bitrate", next_br, NULL);
		}

		std::cout << "[AdaptiveRate] Buffer overrun! Throttled bitrate "
			  << curr << " -> " << next_br << " kbps | Total drops: "
			  << m_dropped_frames.load() << "\n";
	}
}

void GstStreamSink::checkAdaptiveRateRecovery()
{
	if (!m_config.encoder.adaptive_rate) { return; }

	auto now = std::chrono::steady_clock::now();
	double calm_sec = std::chrono::duration<double>(now - m_last_overrun_time).count();
	double since_last_inc = std::chrono::duration<double>(now - m_last_increase_time).count();

	if (calm_sec >= 2.5 && since_last_inc >= 1.0) {
		std::lock_guard<std::mutex> lock(m_rate_mutex);
		int curr = m_current_bitrate.load();

		if (curr < m_config.encoder.bitrate_max_kbps) {
			int next_br = std::min(m_config.encoder.bitrate_max_kbps, curr + 100);
			m_current_bitrate.store(next_br);

			if (m_encoder) {
				g_object_set(m_encoder, "bitrate", next_br, NULL);
			}

			m_last_increase_time = now;
			std::cout << "[AdaptiveRate] Channel calm (" << calm_sec << "s). Bitrate recovered: "
				  << curr << " -> " << next_br << " kbps\n";
		}
	}
}

void GstStreamSink::forceKeyframe()
{
	std::lock_guard<std::mutex> lock(m_pipe_mutex);

	if (!m_pipeline) { return; }

	GstEvent *event = gst_video_event_new_downstream_force_key_unit(
				  GST_CLOCK_TIME_NONE,
				  GST_CLOCK_TIME_NONE,
				  GST_CLOCK_TIME_NONE,
				  TRUE,
				  0
			  );

	if (m_appsrc) {
		gst_element_send_event(GST_ELEMENT(m_appsrc), event);

	} else {
		gst_element_send_event(m_pipeline, event);
	}
}

void GstStreamSink::setBitrate(int bitrate_kbps)
{
	if (bitrate_kbps <= 0) { return; }

	std::lock_guard<std::mutex> rate_lock(m_rate_mutex);
	m_current_bitrate.store(bitrate_kbps);

	std::lock_guard<std::mutex> pipe_lock(m_pipe_mutex);

	if (m_encoder) {
		GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(m_encoder), "bitrate");

		if (pspec) {
			g_object_set(m_encoder, "bitrate", bitrate_kbps, NULL);

		} else {
			GstStructure *s = gst_structure_new("controls", "video_bitrate", G_TYPE_INT, bitrate_kbps * 1000, NULL);

			if (s) {
				g_object_set(m_encoder, "extra-controls", s, NULL);
				gst_structure_free(s);
			}
		}
	}

	std::cout << "[GstStreamSink] Cập nhật bitrate runtime: " << bitrate_kbps << " kbps\n";
}

int GstStreamSink::getCurrentBitrate() const
{
	return m_current_bitrate.load();
}

uint64_t GstStreamSink::getDroppedFrames() const
{
	return m_dropped_frames.load();
}

gboolean GstStreamSink::busCallback(GstBus *bus, GstMessage *msg, gpointer user_data)
{
	(void)bus;
	auto *self = static_cast<GstStreamSink *>(user_data);

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_ERROR: {
			GError *err = nullptr;
			gchar *debug = nullptr;
			gst_message_parse_error(msg, &err, &debug);
			std::cerr << "[GstStreamSink][BUS ERROR] " << (err ? err->message : "")
				  << " (debug: " << (debug ? debug : "") << ")\n";

			if (err) { g_error_free(err); }

			if (debug) { g_free(debug); }

			self->m_is_connected = false;
			self->m_last_error = StreamErrorCode::ERR_GST_PUSH_BUFFER_FAILED;
			break;
		}

	case GST_MESSAGE_EOS:
		std::cerr << "[GstStreamSink][BUS] Đã nhận tín hiệu EOS từ pipeline\n";
		self->m_is_connected = false;
		break;

	default:
		break;
	}

	return TRUE;
}

bool GstStreamSink::pushFrame(const FrameData &frame)
{
	if (!m_enabled.load()) {
		return false;
	}

	std::lock_guard<std::mutex> lock(m_pipe_mutex);

	if (!m_is_connected || !m_appsrc || !m_pipeline) {
		return false;
	}

	// Non-blocking poll bus for error/EOS (PC Task 19)
	if (m_bus) {
		GstMessage *msg = gst_bus_pop_filtered(m_bus, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

		if (msg) {
			if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
				GError *err = nullptr;
				gchar *debug = nullptr;
				gst_message_parse_error(msg, &err, &debug);
				std::cerr << "[GstStreamSink][BUS ERROR] " << (err ? err->message : "")
					  << " (debug: " << (debug ? debug : "") << ")\n";

				if (err) { g_error_free(err); }

				if (debug) { g_free(debug); }

				m_last_error = StreamErrorCode::ERR_GST_PUSH_BUFFER_FAILED;

			} else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
				std::cerr << "[GstStreamSink][BUS] Đã nhận tín hiệu EOS từ pipeline\n";
			}

			gst_message_unref(msg);
			m_is_connected = false;
			return false;
		}
	}

	checkAdaptiveRateRecovery();

	size_t size = frame.data.size();

	if (size == 0) { return false; }

	GstBuffer *buffer = gst_buffer_new_allocate(nullptr, size, nullptr);

	if (!buffer) {
		m_last_error = StreamErrorCode::ERR_GST_PUSH_BUFFER_FAILED;
		return false;
	}

	GstMapInfo map;

	if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
		std::memcpy(map.data, frame.data.data(), size);
		gst_buffer_unmap(buffer, &map);

	} else {
		gst_buffer_unref(buffer);
		return false;
	}

	GST_BUFFER_PTS(buffer) = m_pushed_frames * m_pts_duration;
	GST_BUFFER_DTS(buffer) = GST_BUFFER_PTS(buffer);
	GST_BUFFER_DURATION(buffer) = m_pts_duration;
	m_pushed_frames++;

	GstFlowReturn ret = gst_app_src_push_buffer(m_appsrc, buffer);

	if (ret != GST_FLOW_OK) {
		m_last_error = StreamErrorCode::ERR_GST_PUSH_BUFFER_FAILED;
		m_is_connected = false;
		return false;
	}

	return true;
}

void GstStreamSink::stop()
{
	std::lock_guard<std::mutex> lock(m_pipe_mutex);
	stopLocked();
}

void GstStreamSink::stopLocked()
{
	m_is_connected = false;

	if (m_pipeline) {
		gst_element_set_state(m_pipeline, GST_STATE_NULL);

		if (m_bus_watch_id > 0) {
			g_source_remove(m_bus_watch_id);
			m_bus_watch_id = 0;
		}

		if (m_bus) {
			gst_object_unref(m_bus);
			m_bus = nullptr;
		}

		if (m_appsrc) {
			gst_object_unref(m_appsrc);
			m_appsrc = nullptr;
		}

		if (m_queue) {
			gst_object_unref(m_queue);
			m_queue = nullptr;
		}

		if (m_encoder) {
			gst_object_unref(m_encoder);
			m_encoder = nullptr;
		}

		gst_object_unref(m_pipeline);
		m_pipeline = nullptr;
	}
}

bool GstStreamSink::isConnected() const
{
	return m_is_connected.load();
}

StreamErrorCode GstStreamSink::getLastErrorCode() const
{
	return m_last_error.load();
}

} // namespace drone
