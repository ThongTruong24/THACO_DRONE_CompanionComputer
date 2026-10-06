#include <gtest/gtest.h>
#include "camera_streamer/ai_branch.hpp"
#include <vector>
#include <cmath>

using namespace cc;

TEST(AiBranchTest, BilinearDownscaleSmall)
{
	// 4x4 RGB image with uniform red = 100, green = 50, blue = 200
	const int in_w = 4;
	const int in_h = 4;
	std::vector<uint8_t> src(in_w * in_h * 3);

	for (size_t i = 0; i < src.size(); i += 3) {
		src[i] = 100;     // B
		src[i + 1] = 50;  // G
		src[i + 2] = 200; // R
	}

	const int out_w = 2;
	const int out_h = 2;
	std::vector<uint8_t> dst(out_w * out_h * 3, 0);

	AiBranch::bilinear_downscale_bgr(src.data(), in_w, in_h, dst.data(), out_w, out_h);

	for (size_t i = 0; i < dst.size(); i += 3) {
		EXPECT_EQ(dst[i], 100);
		EXPECT_EQ(dst[i + 1], 50);
		EXPECT_EQ(dst[i + 2], 200);
	}
}

TEST(AiBranchTest, BilinearDownscaleGradient)
{
	// 2x2 image:
	// Pixel (0,0)=0, (1,0)=100
	// Pixel (0,1)=100, (1,1)=200
	const int in_w = 2;
	const int in_h = 2;
	std::vector<uint8_t> src = {
		0, 0, 0,       100, 100, 100,
		100, 100, 100, 200, 200, 200
	};

	const int out_w = 1;
	const int out_h = 1;
	std::vector<uint8_t> dst(3, 0);

	AiBranch::bilinear_downscale_bgr(src.data(), in_w, in_h, dst.data(), out_w, out_h);

	// Center point (0.5, 0.5) should be average of all 4 pixels = 100
	EXPECT_NEAR(dst[0], 100, 2);
	EXPECT_NEAR(dst[1], 100, 2);
	EXPECT_NEAR(dst[2], 100, 2);
}

TEST(AiBranchTest, NearestDownscaleDepth)
{
	// 4x4 depth image with distinct values in each quadrant:
	// Q1: 1000, Q2: 2000
	// Q3: 3000, Q4: 4000
	const int in_w = 4;
	const int in_h = 4;
	std::vector<uint16_t> src(in_w * in_h, 0);

	for (int y = 0; y < in_h; ++y) {
		for (int x = 0; x < in_w; ++x) {
			if (y < 2 && x < 2) {
				src[y * in_w + x] = 1000;

			} else if (y < 2 && x >= 2) {
				src[y * in_w + x] = 2000;

			} else if (y >= 2 && x < 2) {
				src[y * in_w + x] = 3000;

			} else {
				src[y * in_w + x] = 4000;
			}
		}
	}

	const int out_w = 2;
	const int out_h = 2;
	std::vector<uint16_t> dst(out_w * out_h, 0);

	AiBranch::nearest_downscale_depth(src.data(), in_w, in_h, dst.data(), out_w, out_h);

	// Downscaled 2x2 must have exact discrete depth values, never intermediate blended values
	EXPECT_EQ(dst[0], 1000);
	EXPECT_EQ(dst[1], 2000);
	EXPECT_EQ(dst[2], 3000);
	EXPECT_EQ(dst[3], 4000);
}

TEST(AiBranchTest, ScaleIntrinsics)
{
	float in_fx = 600.0f;
	float in_fy = 600.0f;
	float in_cx = 320.0f;
	float in_cy = 240.0f;

	int in_w = 640;
	int in_h = 480;
	int out_w = 320;
	int out_h = 240;

	float out_fx, out_fy, out_cx, out_cy;
	AiBranch::scale_intrinsics(
		in_fx, in_fy, in_cx, in_cy,
		in_w, in_h, out_w, out_h,
		out_fx, out_fy, out_cx, out_cy
	);

	EXPECT_FLOAT_EQ(out_fx, 300.0f);
	EXPECT_FLOAT_EQ(out_fy, 300.0f);
	EXPECT_FLOAT_EQ(out_cx, 160.0f);
	EXPECT_FLOAT_EQ(out_cy, 120.0f);
}

TEST(AiBranchTest, CameraInfoGeneration)
{
	AiBranchConfig cfg;
	cfg.width = 640;
	cfg.height = 480;
	cfg.fps = 10;
	cfg.pool_dir = "/tmp/test_ai_pool_info";

	AiBranch branch(cfg);

	drone::FrameData frame;
	frame.width = 1280;
	frame.height = 720;
	frame.fx = 1000.0f;
	frame.fy = 1000.0f;
	frame.cx = 640.0f;
	frame.cy = 360.0f;

	auto info = branch.get_camera_info(frame);

	EXPECT_EQ(info.width, 640u);
	EXPECT_EQ(info.height, 480u);
	EXPECT_EQ(info.distortion_model, "plumb_bob");

	// Scaled fx: 1000 * (640 / 1280) = 500
	EXPECT_FLOAT_EQ(info.k[0], 500.0f);
	// Scaled fy: 1000 * (480 / 720) = 666.6667
	EXPECT_NEAR(info.k[4], 1000.0f * (480.0f / 720.0f), 0.01f);
	// Scaled cx: 640 * (640 / 1280) = 320
	EXPECT_FLOAT_EQ(info.k[2], 320.0f);
	// Scaled cy: 360 * (480 / 720) = 240
	EXPECT_FLOAT_EQ(info.k[5], 240.0f);
}

TEST(AiBranchTest, SkipWhenNoSubscribersOrDisabled)
{
	AiBranchConfig cfg;
	cfg.enabled = true;
	cfg.width = 320;
	cfg.height = 240;
	cfg.fps = 10;
	cfg.pool_dir = "/tmp/test_ai_pool_subs";

	AiBranch branch(cfg);

	drone::FrameData frame;
	frame.width = 640;
	frame.height = 480;
	frame.data.resize(640 * 480 * 3, 128);

	// Case 1: has_subscribers = false -> should return nullopt
	auto res1 = branch.process_frame(frame, /*has_subscribers=*/false, 1000000ULL);
	EXPECT_FALSE(res1.has_value());

	// Case 2: enabled = false -> should return nullopt
	cfg.enabled = false;
	branch.update_config(cfg);
	auto res2 = branch.process_frame(frame, /*has_subscribers=*/true, 1000000ULL);
	EXPECT_FALSE(res2.has_value());

	// Case 3: enabled = true, has_subscribers = true -> should succeed
	cfg.enabled = true;
	branch.update_config(cfg);
	auto res3 = branch.process_frame(frame, /*has_subscribers=*/true, 1000000ULL);
	EXPECT_TRUE(res3.has_value());
}

TEST(AiBranchTest, RateLimiting)
{
	AiBranchConfig cfg;
	cfg.enabled = true;
	cfg.width = 320;
	cfg.height = 240;
	cfg.fps = 10; // 10 fps -> 100ms (100,000 us) interval
	cfg.pool_dir = "/tmp/test_ai_pool_rate";

	AiBranch branch(cfg);

	drone::FrameData frame;
	frame.width = 640;
	frame.height = 480;
	frame.data.resize(640 * 480 * 3, 128);

	// First frame at t = 1,000,000 us
	auto res1 = branch.process_frame(frame, true, 1000000ULL);
	EXPECT_TRUE(res1.has_value());

	// Second frame at t = 1,050,000 us (+50ms, too soon for 10 fps) -> must be dropped
	auto res2 = branch.process_frame(frame, true, 1050000ULL);
	EXPECT_FALSE(res2.has_value());

	// Third frame at t = 1,110,000 us (+110ms >= 100ms) -> must pass
	auto res3 = branch.process_frame(frame, true, 1110000ULL);
	EXPECT_TRUE(res3.has_value());
}
