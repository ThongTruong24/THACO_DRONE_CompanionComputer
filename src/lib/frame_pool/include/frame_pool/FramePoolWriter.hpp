#pragma once

#include "FramePool.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace cc
{

class FramePoolWriter
{
public:
	FramePoolWriter();
	~FramePoolWriter();

	FramePoolWriter(const FramePoolWriter &) = delete;
	FramePoolWriter &operator=(const FramePoolWriter &) = delete;

	/**
	 * Initializes or re-initializes the frame pool file in dir.
	 * Removes old pool.* files in dir.
	 */
	bool init(const std::string &dir, uint32_t width, uint32_t height,
		  bool has_depth = false, float depth_scale = 0.001f,
		  uint32_t num_slots = kDefaultNumSlots);

	/**
	 * Writes a frame into the next slot using seqlock.
	 * Returns descriptor to publish via ROS 2.
	 */
	FrameReadyDesc write_frame(const uint8_t *color_data, size_t color_size,
				   const uint8_t *depth_data = nullptr, size_t depth_size = 0,
				   uint64_t timestamp = 0);

	void close_pool();

	uint32_t get_generation() const { return _generation; }
	uint32_t get_width() const { return _width; }
	uint32_t get_height() const { return _height; }
	uint32_t get_slot_size() const { return _slot_size; }
	size_t get_total_size() const { return _total_size; }

private:
	void cleanup_old_pools();

	std::string _dir;
	uint32_t _generation{0};
	uint32_t _width{0};
	uint32_t _height{0};
	bool _has_depth{false};
	float _depth_scale{0.001f};
	uint32_t _num_slots{kDefaultNumSlots};
	uint32_t _slot_size{0};
	size_t _total_size{0};

	int _fd{-1};
	uint8_t *_base{nullptr};
	uint64_t _frame_count{0};
};

} // namespace cc
