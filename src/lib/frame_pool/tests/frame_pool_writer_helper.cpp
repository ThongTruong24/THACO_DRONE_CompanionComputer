#include "frame_pool/FramePoolWriter.hpp"
#include <iostream>
#include <vector>

int main(int argc, char **argv)
{
	std::string dir = "/tmp/pool_py_test";
	uint32_t count = 5;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if (arg == "--dir" && i + 1 < argc) { dir = argv[++i]; }

		else if (arg == "--count" && i + 1 < argc) { count = std::stoul(argv[++i]); }
	}

	cc::FramePoolWriter writer;

	if (!writer.init(dir, 64, 48, true, 0.001f, 3)) {
		std::cerr << "Failed to init writer" << std::endl;
		return 1;
	}

	std::vector<uint8_t> color(64 * 48 * 3);
	std::vector<uint16_t> depth(64 * 48);

	for (uint32_t i = 1; i <= count; ++i) {
		std::fill(color.begin(), color.end(), static_cast<uint8_t>(i));
		std::fill(depth.begin(), depth.end(), static_cast<uint16_t>(i * 2));
		auto desc = writer.write_frame(color.data(), color.size(),
					       reinterpret_cast<const uint8_t *>(depth.data()),
					       depth.size() * sizeof(uint16_t),
					       1000ULL * i);
		// Print desc: gen slot seq
		std::cout << desc.generation << " " << static_cast<int>(desc.slot) << " " << desc.seq << std::endl;
	}

	return 0;
}
