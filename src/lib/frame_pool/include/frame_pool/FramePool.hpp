#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace cc
{

#pragma pack(push, 1)

struct FramePoolHeader {
	char magic[4];             // "CCFP"
	uint32_t version;          // 1
	uint32_t generation;       // Boot ms or monotonically increasing
	uint32_t width;            // Frame width (e.g. 640)
	uint32_t height;           // Frame height (e.g. 480)
	char encoding_color[8];    // "bgr8\0\0\0\0"
	uint8_t has_depth;         // 1 if depth present, 0 otherwise
	uint8_t reserved[3];       // Padding
	char encoding_depth[8];    // "16UC1\0\0\0"
	float depth_scale;         // Depth scale in meters (e.g. 0.001f)
	uint32_t slot_size;        // Size of each slot aligned to 64 bytes
	uint32_t num_slots;        // Number of slots (typically 3)
	uint8_t pad[12];           // Reserved / alignment to 64 bytes
};

static_assert(sizeof(FramePoolHeader) == 64, "FramePoolHeader must be exactly 64 bytes");

struct FrameSlotHeader {
	uint64_t seq;              // Seqlock: odd = writing, even = valid
	uint64_t timestamp;        // Microseconds (hrt_absolute_time)
	uint32_t color_offset;     // Offset of color data from start of slot (usually 64)
	uint32_t color_size;       // Size of color data (width * height * 3)
	uint32_t depth_offset;     // Offset of depth data from start of slot
	uint32_t depth_size;       // Size of depth data (width * height * 2)
	uint8_t pad[32];           // Reserved / alignment to 64 bytes
};

static_assert(sizeof(FrameSlotHeader) == 64, "FrameSlotHeader must be exactly 64 bytes");

#pragma pack(pop)

constexpr uint32_t kDefaultNumSlots = 3;
constexpr const char *kFramePoolMagic = "CCFP";
constexpr uint32_t kFramePoolVersion = 1;

struct FrameReadyDesc {
	uint64_t timestamp;
	uint32_t generation;
	uint8_t slot;
	uint32_t seq;
};

} // namespace cc
