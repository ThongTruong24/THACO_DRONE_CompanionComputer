#include "frame_pool/FramePoolWriter.hpp"
#include "hrt/hrt.hpp"

#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cc
{

FramePoolWriter::FramePoolWriter() = default;

FramePoolWriter::~FramePoolWriter()
{
	close_pool();
}

void FramePoolWriter::close_pool()
{
	if (_base != nullptr && _base != MAP_FAILED) {
		munmap(_base, _total_size);
		_base = nullptr;
	}

	if (_fd >= 0) {
		::close(_fd);
		_fd = -1;
	}
}

void FramePoolWriter::cleanup_old_pools()
{
	try {
		std::string current_name = "pool." + std::to_string(_generation);

		for (const auto &entry : std::filesystem::directory_iterator(_dir)) {
			if (!entry.is_regular_file()) { continue; }

			std::string filename = entry.path().filename().string();

			if (filename.rfind("pool.", 0) == 0 && filename != current_name) {
				std::error_code ec;
				std::filesystem::remove(entry.path(), ec);
			}
		}

	} catch (...) {
		// Ignore directory scanning errors
	}
}

bool FramePoolWriter::init(const std::string &dir, uint32_t width, uint32_t height,
			   bool has_depth, float depth_scale, uint32_t num_slots)
{
	close_pool();

	_dir = dir;
	_width = width;
	_height = height;
	_has_depth = has_depth;
	_depth_scale = depth_scale;
	_num_slots = (num_slots > 0) ? num_slots : kDefaultNumSlots;

	std::error_code ec;
	std::filesystem::create_directories(_dir, ec);

	// generation = max(gen + 1, số ms từ lúc boot);
	struct timespec ts;
	clock_gettime(CLOCK_BOOTTIME, &ts);
	uint32_t boot_ms = static_cast<uint32_t>(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
	_generation = std::max(_generation + 1, boot_ms);

	size_t color_size = static_cast<size_t>(_width) * _height * 3;
	size_t depth_size = _has_depth ? (static_cast<size_t>(_width) * _height * 2) : 0;
	size_t raw_slot = sizeof(FrameSlotHeader) + color_size + depth_size;
	_slot_size = static_cast<uint32_t>((raw_slot + 63) / 64 * 64);
	_total_size = sizeof(FramePoolHeader) + static_cast<size_t>(_slot_size) * _num_slots;

	std::string tmp_path = _dir + "/pool." + std::to_string(_generation) + ".tmp";
	std::string final_path = _dir + "/pool." + std::to_string(_generation);

	_fd = ::open(tmp_path.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);

	if (_fd < 0) { return false; }

	if (ftruncate(_fd, _total_size) != 0) {
		::close(_fd);
		_fd = -1;
		std::filesystem::remove(tmp_path, ec);
		return false;
	}

	_base = static_cast<uint8_t *>(mmap(nullptr, _total_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0));

	if (_base == MAP_FAILED) {
		::close(_fd);
		_fd = -1;
		_base = nullptr;
		std::filesystem::remove(tmp_path, ec);
		return false;
	}

	std::memset(_base, 0, _total_size);

	auto *hdr = reinterpret_cast<FramePoolHeader *>(_base);
	std::memcpy(hdr->magic, kFramePoolMagic, 4);
	hdr->version = kFramePoolVersion;
	hdr->generation = _generation;
	hdr->width = _width;
	hdr->height = _height;
	std::strncpy(hdr->encoding_color, "bgr8", sizeof(hdr->encoding_color));
	hdr->has_depth = _has_depth ? 1 : 0;
	std::strncpy(hdr->encoding_depth, _has_depth ? "16UC1" : "", sizeof(hdr->encoding_depth));
	hdr->depth_scale = _depth_scale;
	hdr->slot_size = _slot_size;
	hdr->num_slots = _num_slots;

	msync(_base, sizeof(FramePoolHeader), MS_SYNC);
	fsync(_fd);

	if (std::rename(tmp_path.c_str(), final_path.c_str()) != 0) {
		close_pool();
		std::filesystem::remove(tmp_path, ec);
		return false;
	}

	cleanup_old_pools();
	_frame_count = 0;
	return true;
}

FrameReadyDesc FramePoolWriter::write_frame(const uint8_t *color_data, size_t color_size,
		const uint8_t *depth_data, size_t depth_size,
		uint64_t timestamp)
{
	if (!_base || _slot_size == 0 || _num_slots == 0) {
		return {};
	}

	if (timestamp == 0) {
		timestamp = hrt_absolute_time();
	}

	uint8_t slot_idx = static_cast<uint8_t>(_frame_count % _num_slots);
	size_t slot_offset = sizeof(FramePoolHeader) + static_cast<size_t>(slot_idx) * _slot_size;
	auto *slot_hdr = reinterpret_cast<FrameSlotHeader *>(_base + slot_offset);

	uint64_t odd_seq = _frame_count * 2 + 1;
	std::atomic_ref<uint64_t> seq_ref(slot_hdr->seq);
	seq_ref.store(odd_seq, std::memory_order_release);
	std::atomic_thread_fence(std::memory_order_release);

	slot_hdr->timestamp = timestamp;
	slot_hdr->color_offset = sizeof(FrameSlotHeader);
	slot_hdr->color_size = static_cast<uint32_t>(color_size);

	uint8_t *data_ptr = _base + slot_offset + sizeof(FrameSlotHeader);

	if (color_data && color_size > 0) {
		std::memcpy(data_ptr, color_data, color_size);
	}

	if (_has_depth && depth_data && depth_size > 0) {
		slot_hdr->depth_offset = static_cast<uint32_t>(sizeof(FrameSlotHeader) + color_size);
		slot_hdr->depth_size = static_cast<uint32_t>(depth_size);
		std::memcpy(data_ptr + color_size, depth_data, depth_size);

	} else {
		slot_hdr->depth_offset = 0;
		slot_hdr->depth_size = 0;
	}

	uint64_t even_seq = _frame_count * 2 + 2;
	seq_ref.store(even_seq, std::memory_order_release);

	FrameReadyDesc desc;
	desc.timestamp = timestamp;
	desc.generation = _generation;
	desc.slot = slot_idx;
	desc.seq = static_cast<uint32_t>(even_seq);

	_frame_count++;
	return desc;
}

} // namespace cc
