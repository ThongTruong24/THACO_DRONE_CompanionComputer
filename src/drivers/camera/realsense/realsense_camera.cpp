#include "realsense_camera.hpp"
#include <iostream>
#include <cstring>

namespace drone
{

#if defined(HAVE_REALSENSE) && (HAVE_REALSENSE == 1)

RealSenseCamera::RealSenseCamera()
	: m_is_streaming(false),
	  m_is_healthy(false),
	  m_last_error(StreamErrorCode::SUCCESS),
	  m_device_name("Intel RealSense D400"),
	  m_frame_counter(0) {}

RealSenseCamera::~RealSenseCamera()
{
	stopStream();
}

bool RealSenseCamera::initialize(const CameraConfig &config)
{
	m_config = config;
	m_last_error = StreamErrorCode::SUCCESS;

	try {
		rs2::context ctx;
		auto devices = ctx.query_devices();

		if (devices.size() == 0) {
			std::cerr << "[RealSenseCamera] KHÔNG TÌM THẤY THIẾT BỊ REALSENSE NÀO TRÊN CỔNG USB!\n";
			m_last_error = StreamErrorCode::ERR_DEVICE_NOT_FOUND;
			return false;
		}

		auto dev = devices[0];
		m_device_name = dev.get_info(RS2_CAMERA_INFO_NAME);
		std::cout << "[RealSenseCamera] Đã tìm thấy thiết bị: " << m_device_name
			  << " (Serial: " << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << ")\n";

		// Cấu hình Emitter
		for (auto &&sensor : dev.query_sensors()) {
			if (sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
				sensor.set_option(RS2_OPTION_EMITTER_ENABLED, m_config.realsense.enable_emitter ? 1.0f : 0.0f);
			}
		}

		m_pipeline = std::make_unique<rs2::pipeline>();
		m_rs_config = std::make_unique<rs2::config>();

		if (!m_config.realsense.serial_number.empty()) {
			m_rs_config->enable_device(m_config.realsense.serial_number);
		}

		// Cấu hình luồng màu RGB
		m_rs_config->enable_stream(
			RS2_STREAM_COLOR,
			m_config.video.width,
			m_config.video.height,
			RS2_FORMAT_BGR8,
			m_config.video.fps
		);

		// Chế độ profile
		if (m_config.realsense.profile_mode == "rgb_depth" || m_config.realsense.profile_mode == "rgb_depth_pointcloud") {
			std::cout << "[RealSenseCamera] Kích hoạt chế độ RGB + Depth ("
				  << m_config.realsense.depth_width << "x" << m_config.realsense.depth_height
				  << "@" << m_config.realsense.depth_fps << "fps)...\n";
			m_rs_config->enable_stream(
				RS2_STREAM_DEPTH,
				m_config.realsense.depth_width,
				m_config.realsense.depth_height,
				RS2_FORMAT_Z16,
				m_config.realsense.depth_fps
			);

		} else {
			std::cout <<
				  "[RealSenseCamera] Chế độ RGB_ONLY: ĐÃ TẮT HOÀN TOÀN DEPTH/IR/POINTCLOUD -> Dành 100% băng thông USB 3.0 cho RGB!\n";
		}

		m_is_healthy = true;
		return true;

	} catch (const rs2::error &e) {
		std::cerr << "[RealSenseCamera][ERROR] Khởi tạo thất bại: " << e.what() << "\n";
		m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
		m_is_healthy = false;
		return false;
	}
}

bool RealSenseCamera::startStream()
{
	if (m_is_streaming) { return true; }

	if (!m_pipeline || !m_rs_config) { return false; }

	try {
		m_pipeline->start(*m_rs_config);
		m_is_streaming = true;
		m_is_healthy = true;
		std::cout << "[RealSenseCamera] ✓ Đã khởi động luồng phần cứng RealSense ("
			  << m_config.video.width << "x" << m_config.video.height << "@" << m_config.video.fps << "fps)\n";
		return true;

	} catch (const rs2::error &e) {
		std::cerr << "[RealSenseCamera][ERROR] Không thể start stream: " << e.what() << "\n";
		m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
		m_is_healthy = false;
		return false;
	}
}

void RealSenseCamera::stopStream()
{
	if (m_is_streaming && m_pipeline) {
		try {
			m_pipeline->stop();

		} catch (...) {}

		m_is_streaming = false;
	}

	m_pipeline.reset();
	m_rs_config.reset();
}

bool RealSenseCamera::captureFrame(FrameData &out_frame)
{
	if (!m_is_streaming || !m_pipeline) { return false; }

	try {
		rs2::frameset frames = m_pipeline->wait_for_frames(100);
		rs2::video_frame color_frame = frames.get_color_frame();

		if (!color_frame) {
			m_last_error = StreamErrorCode::ERR_CAPTURE_TIMEOUT;
			return false;
		}

		int width = color_frame.get_width();
		int height = color_frame.get_height();
		const uint8_t *raw_data = static_cast<const uint8_t *>(color_frame.get_data());
		size_t size = static_cast<size_t>(width) * height * 3;

		out_frame.width = width;
		out_frame.height = height;
		out_frame.format = PixelFormat::BGR8;
		out_frame.frame_number = ++m_frame_counter;
		out_frame.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
						 std::chrono::steady_clock::now().time_since_epoch()
					 ).count();

		if (out_frame.data.size() != size) {
			out_frame.data.resize(size);
		}

		std::memcpy(out_frame.data.data(), raw_data, size);

		m_is_healthy = true;
		m_last_error = StreamErrorCode::SUCCESS;
		return true;

	} catch (const rs2::error &e) {
		std::string err_msg = e.what();

		if (err_msg.find("UVC") != std::string::npos || err_msg.find("EPROTO") != std::string::npos || err_msg.find("-71") != std::string::npos) {
			m_last_error = StreamErrorCode::ERR_USB_IO_ERROR;

		} else {
			m_last_error = StreamErrorCode::ERR_CAPTURE_TIMEOUT;
		}

		m_is_healthy = false;
		return false;
	}
}

bool RealSenseCamera::isAvailable() const
{
	try {
		rs2::context ctx;
		auto devs = ctx.query_devices();
		return devs.size() > 0;

	} catch (...) {
		return false;
	}
}

bool RealSenseCamera::isHealthy() const
{
	return m_is_healthy.load();
}

std::string RealSenseCamera::getDeviceName() const
{
	return m_device_name;
}

StreamErrorCode RealSenseCamera::getLastErrorCode() const
{
	return m_last_error.load();
}

#else

// ----------------------------------------------------------------------------
// Stub Implementation khi không có librealsense2 (trên máy dev WSL2 không có ROS2/D435)
// ----------------------------------------------------------------------------
RealSenseCamera::RealSenseCamera()
	: m_is_streaming(false),
	  m_is_healthy(false),
	  m_last_error(StreamErrorCode::ERR_DEVICE_NOT_FOUND),
	  m_device_name("Intel RealSense (Stub Mode - Host without RealSense Driver)"),
	  m_frame_counter(0) {}

RealSenseCamera::~RealSenseCamera() {}

bool RealSenseCamera::initialize(const CameraConfig &config)
{
	m_config = config;
	std::cerr << "[RealSenseCamera][WARN] librealsense2 không có trên host này. Kích hoạt fallback V4L2/Standby!\n";
	m_last_error = StreamErrorCode::ERR_DEVICE_NOT_FOUND;
	return false;
}

bool RealSenseCamera::startStream() { return false; }
void RealSenseCamera::stopStream() {}
bool RealSenseCamera::captureFrame(FrameData & /*out_frame*/) { return false; }
bool RealSenseCamera::isAvailable() const { return false; }
bool RealSenseCamera::isHealthy() const { return false; }
std::string RealSenseCamera::getDeviceName() const { return m_device_name; }
StreamErrorCode RealSenseCamera::getLastErrorCode() const { return m_last_error.load(); }

#endif

} // namespace drone
