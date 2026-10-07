#include "neon_processor.hpp"
#include <algorithm>
#include <cstring>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

namespace drone
{

void NeonFrameProcessor::process(FrameData &frame)
{
	if (m_rotation.load() == 180 && !frame.data.empty() && frame.width > 0 && frame.height > 0) {
		rotate180_inplace(frame.data.data(), frame.width, frame.height);
	}
}

void NeonFrameProcessor::rotate180_inplace(uint8_t *data, int width, int height)
{
	// Với ảnh BGR8 / RGB8 (3 bytes mỗi pixel):
	// Xoay 180° = Đảo ngược toàn bộ pixel từ đầu đến cuối
	const size_t total_pixels = static_cast<size_t>(width) * height;

	if (total_pixels == 0) { return; }

	uint8_t *ptr_front = data;
	uint8_t *ptr_back = data + (total_pixels - 1) * 3;

	while (ptr_front < ptr_back) {
		// Swap 3 bytes BGR
		uint8_t b0 = ptr_front[0];
		uint8_t g0 = ptr_front[1];
		uint8_t r0 = ptr_front[2];

		ptr_front[0] = ptr_back[0];
		ptr_front[1] = ptr_back[1];
		ptr_front[2] = ptr_back[2];

		ptr_back[0] = b0;
		ptr_back[1] = g0;
		ptr_back[2] = r0;

		ptr_front += 3;
		ptr_back -= 3;
	}
}

} // namespace drone
