#include "v4l2_camera.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include <unistd.h>

namespace drone
{

bool V4L2Camera::isAvailable() const
{
	const std::string &path = m_config.v4l2.device_path.empty() ? "/dev/video0" : m_config.v4l2.device_path;
	return access(path.c_str(), R_OK | W_OK) == 0;
}

V4L2Camera::V4L2Camera()
	: m_fd(-1),
	  m_is_streaming(false),
	  m_is_healthy(false),
	  m_last_error(StreamErrorCode::SUCCESS),
	  m_device_name("V4L2 USB Camera"),
	  m_frame_counter(0) {}

V4L2Camera::~V4L2Camera()
{
	stopStream();

	if (m_fd >= 0) {
		close(m_fd);
	}
}

bool V4L2Camera::initialize(const CameraConfig &config)
{
	m_config = config;
	m_last_error = StreamErrorCode::SUCCESS;

	m_fd = open(m_config.v4l2.device_path.c_str(), O_RDWR | O_NONBLOCK, 0);

	if (m_fd < 0) {
		std::cerr << "[V4L2Camera][ERROR] Không thể mở thiết bị: " << m_config.v4l2.device_path << "\n";
		m_last_error = StreamErrorCode::ERR_DEVICE_NOT_FOUND;
		m_is_healthy = false;
		return false;
	}

	struct v4l2_capability cap;

	if (ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
		std::cerr << "[V4L2Camera][ERROR] Truy vấn capability thất bại\n";
		m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
		return false;
	}

	m_device_name = reinterpret_cast<char *>(cap.card);

	struct v4l2_format fmt;
	std::memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = m_config.video.width;
	fmt.fmt.pix.height = m_config.video.height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

	if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) < 0) {
		// Thử fallback sang YUYV
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

		if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) < 0) {
			std::cerr << "[V4L2Camera][ERROR] Cài đặt định dạng thất bại\n";
			m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
			return false;
		}
	}

	struct v4l2_requestbuffers req;

	std::memset(&req, 0, sizeof(req));

	req.count = m_config.v4l2.num_buffers > 0 ? m_config.v4l2.num_buffers : 4;

	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0) {
		std::cerr << "[V4L2Camera][ERROR] REQBUFS thất bại\n";
		m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
		return false;
	}

	m_buffers.resize(req.count);

	for (size_t i = 0; i < req.count; ++i) {
		struct v4l2_buffer buf;
		std::memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (ioctl(m_fd, VIDIOC_QUERYBUF, &buf) < 0) {
			m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
			return false;
		}

		m_buffers[i].length = buf.length;
		m_buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, buf.m.offset);

		if (m_buffers[i].start == MAP_FAILED) {
			m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
			return false;
		}
	}

	m_is_healthy = true;
	std::cout << "[V4L2Camera] ✓ Khởi tạo thành công: " << m_device_name
		  << " (" << m_config.video.width << "x" << m_config.video.height << ")\n";
	return true;
}

bool V4L2Camera::startStream()
{
	if (m_is_streaming) { return true; }

	if (m_fd < 0) { return false; }

	for (size_t i = 0; i < m_buffers.size(); ++i) {
		struct v4l2_buffer buf;
		std::memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (ioctl(m_fd, VIDIOC_QBUF, &buf) < 0) {
			m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
			return false;
		}
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
		m_last_error = StreamErrorCode::ERR_DEVICE_OPEN_FAILED;
		return false;
	}

	m_is_streaming = true;
	m_is_healthy = true;
	return true;
}

void V4L2Camera::stopStream()
{
	if (m_is_streaming && m_fd >= 0) {
		enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ioctl(m_fd, VIDIOC_STREAMOFF, &type);
		m_is_streaming = false;
	}
}

bool V4L2Camera::captureFrame(FrameData &out_frame)
{
	if (!m_is_streaming || m_fd < 0) { return false; }

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_fd, &fds);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; // 100ms timeout

	int r = select(m_fd + 1, &fds, NULL, NULL, &tv);

	if (r <= 0) {
		m_last_error = StreamErrorCode::ERR_CAPTURE_TIMEOUT;
		return false;
	}

	struct v4l2_buffer buf;

	std::memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(m_fd, VIDIOC_DQBUF, &buf) < 0) {
		m_last_error = StreamErrorCode::ERR_USB_IO_ERROR;
		return false;
	}

	out_frame.width = m_config.video.width;
	out_frame.height = m_config.video.height;
	out_frame.format = PixelFormat::BGR8;
	out_frame.frame_number = ++m_frame_counter;
	out_frame.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
					 std::chrono::steady_clock::now().time_since_epoch()
				 ).count();

	if (out_frame.data.size() != buf.bytesused) {
		out_frame.data.resize(buf.bytesused);
	}

	std::memcpy(out_frame.data.data(), m_buffers[buf.index].start, buf.bytesused);

	ioctl(m_fd, VIDIOC_QBUF, &buf);

	m_is_healthy = true;
	m_last_error = StreamErrorCode::SUCCESS;
	return true;
}

bool V4L2Camera::isHealthy() const
{
	return m_is_healthy.load();
}

std::string V4L2Camera::getDeviceName() const
{
	return m_device_name;
}

StreamErrorCode V4L2Camera::getLastErrorCode() const
{
	return m_last_error.load();
}

} // namespace drone
