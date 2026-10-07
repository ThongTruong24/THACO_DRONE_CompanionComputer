#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <frame_pool/FramePool.hpp>
#include <frame_pool/FramePoolWriter.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include "ICameraDevice.hpp"

namespace cc
{

struct AiBranchConfig {
	bool enabled{true};
	int width{640};
	int height{480};
	int fps{10};
	std::string pool_dir{"/run/frame_pool"};
};

class AiBranch
{
public:
	explicit AiBranch(AiBranchConfig config = {});
	~AiBranch() = default;

	bool init(const AiBranchConfig &config);
	void update_config(const AiBranchConfig &config);
	const AiBranchConfig &config() const { return config_; }

	/**
	 * Process frame for AI: downscales, writes to FramePool, returns descriptor.
	 * If rate limit, not enabled, or has_subscribers is false, returns std::nullopt.
	 */
	std::optional<FrameReadyDesc> process_frame(
		const drone::FrameData &frame,
		bool has_subscribers,
		uint64_t current_time_us);

	sensor_msgs::msg::CameraInfo get_camera_info(const drone::FrameData &frame) const;

	// Pure scaling functions for testability
	static void bilinear_downscale_bgr(
		const uint8_t *src, int in_w, int in_h,
		uint8_t *dst, int out_w, int out_h);

	static void nearest_downscale_depth(
		const uint16_t *src, int in_w, int in_h,
		uint16_t *dst, int out_w, int out_h);

	static void scale_intrinsics(
		float in_fx, float in_fy, float in_cx, float in_cy,
		int in_w, int in_h, int out_w, int out_h,
		float &out_fx, float &out_fy, float &out_cx, float &out_cy);

private:
	AiBranchConfig config_;
	FramePoolWriter pool_writer_;
	uint64_t last_ai_time_us_{0};

	std::vector<uint8_t> scaled_color_buf_;
	std::vector<uint16_t> scaled_depth_buf_;
};

} // namespace cc
