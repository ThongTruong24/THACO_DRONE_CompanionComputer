#include "camera_streamer/ai_branch.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace cc
{

AiBranch::AiBranch(AiBranchConfig config)
	: config_(std::move(config))
{
	init(config_);
}

bool AiBranch::init(const AiBranchConfig &config)
{
	config_ = config;
	last_ai_time_us_ = 0;

	size_t color_bytes = static_cast<size_t>(config_.width) * config_.height * 3;
	scaled_color_buf_.resize(color_bytes);

	size_t depth_pixels = static_cast<size_t>(config_.width) * config_.height;
	scaled_depth_buf_.resize(depth_pixels);

	return pool_writer_.init(
		       config_.pool_dir,
		       config_.width,
		       config_.height,
		       true,    // has_depth supported in pool slots
		       0.001f   // default 1mm depth scale
	       );
}

void AiBranch::update_config(const AiBranchConfig &config)
{
	bool dim_changed = (config.width != config_.width ||
			    config.height != config_.height ||
			    config.pool_dir != config_.pool_dir);
	config_ = config;

	if (dim_changed) {
		init(config);
	}
}

void AiBranch::bilinear_downscale_bgr(
	const uint8_t *src, int in_w, int in_h,
	uint8_t *dst, int out_w, int out_h)
{
	if (!src || !dst || in_w <= 0 || in_h <= 0 || out_w <= 0 || out_h <= 0) {
		return;
	}

	if (in_w == out_w && in_h == out_h) {
		std::memcpy(dst, src, static_cast<size_t>(out_w) * out_h * 3);
		return;
	}

	// ponytail: NEON optimization when profiler shows bottleneck
	const float scale_x = static_cast<float>(in_w) / static_cast<float>(out_w);
	const float scale_y = static_cast<float>(in_h) / static_cast<float>(out_h);

	for (int y = 0; y < out_h; ++y) {
		float gy = (static_cast<float>(y) + 0.5f) * scale_y - 0.5f;

		if (gy < 0.0f) { gy = 0.0f; }

		int y0 = static_cast<int>(gy);
		int y1 = std::min(y0 + 1, in_h - 1);
		float dy = gy - static_cast<float>(y0);
		float w_y0 = 1.0f - dy;
		float w_y1 = dy;

		uint8_t *dst_row = dst + (static_cast<size_t>(y) * out_w * 3);
		const uint8_t *src_row0 = src + (static_cast<size_t>(y0) * in_w * 3);
		const uint8_t *src_row1 = src + (static_cast<size_t>(y1) * in_w * 3);

		for (int x = 0; x < out_w; ++x) {
			float gx = (static_cast<float>(x) + 0.5f) * scale_x - 0.5f;

			if (gx < 0.0f) { gx = 0.0f; }

			int x0 = static_cast<int>(gx);
			int x1 = std::min(x0 + 1, in_w - 1);
			float dx = gx - static_cast<float>(x0);

			float w00 = w_y0 * (1.0f - dx);
			float w01 = w_y0 * dx;
			float w10 = w_y1 * (1.0f - dx);
			float w11 = w_y1 * dx;

			const uint8_t *p00 = src_row0 + x0 * 3;
			const uint8_t *p01 = src_row0 + x1 * 3;
			const uint8_t *p10 = src_row1 + x0 * 3;
			const uint8_t *p11 = src_row1 + x1 * 3;

			uint8_t *d = dst_row + x * 3;

			for (int c = 0; c < 3; ++c) {
				float val = w00 * p00[c] + w01 * p01[c] + w10 * p10[c] + w11 * p11[c];
				d[c] = static_cast<uint8_t>(std::clamp(std::round(val), 0.0f, 255.0f));
			}
		}
	}
}

void AiBranch::nearest_downscale_depth(
	const uint16_t *src, int in_w, int in_h,
	uint16_t *dst, int out_w, int out_h)
{
	if (!src || !dst || in_w <= 0 || in_h <= 0 || out_w <= 0 || out_h <= 0) {
		return;
	}

	if (in_w == out_w && in_h == out_h) {
		std::memcpy(dst, src, static_cast<size_t>(out_w) * out_h * sizeof(uint16_t));
		return;
	}

	// ponytail: NEON optimization when profiler shows bottleneck
	const float scale_x = static_cast<float>(in_w) / static_cast<float>(out_w);
	const float scale_y = static_cast<float>(in_h) / static_cast<float>(out_h);

	for (int y = 0; y < out_h; ++y) {
		int sy = std::clamp(static_cast<int>((static_cast<float>(y) + 0.5f) * scale_y), 0, in_h - 1);
		const uint16_t *src_row = src + static_cast<size_t>(sy) * in_w;
		uint16_t *dst_row = dst + static_cast<size_t>(y) * out_w;

		for (int x = 0; x < out_w; ++x) {
			int sx = std::clamp(static_cast<int>((static_cast<float>(x) + 0.5f) * scale_x), 0, in_w - 1);
			dst_row[x] = src_row[sx];
		}
	}
}

void AiBranch::scale_intrinsics(
	float in_fx, float in_fy, float in_cx, float in_cy,
	int in_w, int in_h, int out_w, int out_h,
	float &out_fx, float &out_fy, float &out_cx, float &out_cy)
{
	if (in_w <= 0 || in_h <= 0 || out_w <= 0 || out_h <= 0) {
		out_fx = in_fx;
		out_fy = in_fy;
		out_cx = in_cx;
		out_cy = in_cy;
		return;
	}

	float sx = static_cast<float>(out_w) / static_cast<float>(in_w);
	float sy = static_cast<float>(out_h) / static_cast<float>(in_h);

	out_fx = in_fx * sx;
	out_fy = in_fy * sy;
	out_cx = in_cx * sx;
	out_cy = in_cy * sy;
}

sensor_msgs::msg::CameraInfo AiBranch::get_camera_info(const drone::FrameData &frame) const
{
	sensor_msgs::msg::CameraInfo info;
	info.width = config_.width;
	info.height = config_.height;
	info.distortion_model = "plumb_bob";
	info.d = {0.0, 0.0, 0.0, 0.0, 0.0};

	int src_w = frame.width > 0 ? frame.width : config_.width;
	int src_h = frame.height > 0 ? frame.height : config_.height;

	float in_fx = frame.fx > 0.0f ? frame.fx : static_cast<float>(src_w);
	float in_fy = frame.fy > 0.0f ? frame.fy : static_cast<float>(src_w);
	float in_cx = frame.cx > 0.0f ? frame.cx : (static_cast<float>(src_w) * 0.5f);
	float in_cy = frame.cy > 0.0f ? frame.cy : (static_cast<float>(src_h) * 0.5f);

	float out_fx, out_fy, out_cx, out_cy;
	scale_intrinsics(in_fx, in_fy, in_cx, in_cy,
			 src_w, src_h,
			 config_.width, config_.height,
			 out_fx, out_fy, out_cx, out_cy);

	// K: 3x3 intrinsic matrix [fx 0 cx; 0 fy cy; 0 0 1]
	info.k[0] = out_fx; info.k[1] = 0.0;    info.k[2] = out_cx;
	info.k[3] = 0.0;    info.k[4] = out_fy; info.k[5] = out_cy;
	info.k[6] = 0.0;    info.k[7] = 0.0;    info.k[8] = 1.0;

	// R: 3x3 identity rectification matrix
	info.r[0] = 1.0; info.r[1] = 0.0; info.r[2] = 0.0;
	info.r[3] = 0.0; info.r[4] = 1.0; info.r[5] = 0.0;
	info.r[6] = 0.0; info.r[7] = 0.0; info.r[8] = 1.0;

	// P: 3x4 projection matrix [fx' 0 cx' Tx; 0 fy' cy' Ty; 0 0 1 0]
	info.p[0] = out_fx; info.p[1] = 0.0;    info.p[2] = out_cx; info.p[3] = 0.0;
	info.p[4] = 0.0;    info.p[5] = out_fy; info.p[6] = out_cy; info.p[7] = 0.0;
	info.p[8] = 0.0;    info.p[9] = 0.0;    info.p[10] = 1.0;  info.p[11] = 0.0;

	return info;
}

std::optional<FrameReadyDesc> AiBranch::process_frame(
	const drone::FrameData &frame,
	bool has_subscribers,
	uint64_t current_time_us)
{
	if (!config_.enabled || !has_subscribers) {
		return std::nullopt;
	}

	if (frame.data.empty() || frame.width <= 0 || frame.height <= 0) {
		return std::nullopt;
	}

	// Rate limiting by ai_fps
	if (config_.fps > 0 && last_ai_time_us_ > 0) {
		uint64_t min_interval_us = 1000000ULL / static_cast<uint64_t>(config_.fps);

		if (current_time_us < last_ai_time_us_ + min_interval_us) {
			return std::nullopt;
		}
	}

	size_t color_bytes = static_cast<size_t>(config_.width) * config_.height * 3;

	if (scaled_color_buf_.size() != color_bytes) {
		scaled_color_buf_.resize(color_bytes);
	}

	bilinear_downscale_bgr(
		frame.data.data(), frame.width, frame.height,
		scaled_color_buf_.data(), config_.width, config_.height
	);

	const uint8_t *depth_ptr = nullptr;
	size_t depth_bytes = 0;

	if (frame.has_depth && !frame.depth_data.empty()) {
		size_t depth_pixels = static_cast<size_t>(config_.width) * config_.height;

		if (scaled_depth_buf_.size() != depth_pixels) {
			scaled_depth_buf_.resize(depth_pixels);
		}

		nearest_downscale_depth(
			frame.depth_data.data(), frame.width, frame.height,
			scaled_depth_buf_.data(), config_.width, config_.height
		);

		depth_ptr = reinterpret_cast<const uint8_t *>(scaled_depth_buf_.data());
		depth_bytes = depth_pixels * sizeof(uint16_t);
	}

	FrameReadyDesc desc = pool_writer_.write_frame(
				      scaled_color_buf_.data(), scaled_color_buf_.size(),
				      depth_ptr, depth_bytes,
				      current_time_us
			      );

	last_ai_time_us_ = current_time_us;
	return desc;
}

} // namespace cc
