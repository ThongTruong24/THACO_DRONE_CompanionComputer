#include <gtest/gtest.h>
#include "frame_pool/FramePool.hpp"
#include "frame_pool/FramePoolWriter.hpp"

#include <filesystem>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace cc;

class FramePoolTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		test_dir_ = (std::filesystem::temp_directory_path() / ("pool_test_" + std::to_string(getpid()))).string();
		std::filesystem::create_directories(test_dir_);
	}

	void TearDown() override
	{
		std::filesystem::remove_all(test_dir_);
	}

	std::string test_dir_;
};

TEST_F(FramePoolTest, HeaderSizeIs64Bytes)
{
	EXPECT_EQ(sizeof(FramePoolHeader), 64u);
	EXPECT_EQ(sizeof(FrameSlotHeader), 64u);
}

TEST_F(FramePoolTest, WriterCreatesFileAndDeletesOldGeneration)
{
	FramePoolWriter writer;
	ASSERT_TRUE(writer.init(test_dir_, 640, 480, false, 0.001f, 3));
	uint32_t gen1 = writer.get_generation();

	std::string path1 = test_dir_ + "/pool." + std::to_string(gen1);
	EXPECT_TRUE(std::filesystem::exists(path1));

	std::vector<uint8_t> dummy_color(640 * 480 * 3, 42);
	auto desc1 = writer.write_frame(dummy_color.data(), dummy_color.size());
	EXPECT_EQ(desc1.generation, gen1);
	EXPECT_EQ(desc1.slot, 0u);
	EXPECT_EQ(desc1.seq, 2u);

	// Re-init with new generation
	ASSERT_TRUE(writer.init(test_dir_, 640, 480, false, 0.001f, 3));
	uint32_t gen2 = writer.get_generation();
	EXPECT_GT(gen2, gen1);

	std::string path2 = test_dir_ + "/pool." + std::to_string(gen2);
	EXPECT_TRUE(std::filesystem::exists(path2));
	EXPECT_FALSE(std::filesystem::exists(path1)); // Old generation deleted
}

TEST_F(FramePoolTest, SeqlockWritingAndSlotOverwriting)
{
	FramePoolWriter writer;
	ASSERT_TRUE(writer.init(test_dir_, 640, 480, false, 0.001f, 3));
	uint32_t gen = writer.get_generation();
	std::string pool_path = test_dir_ + "/pool." + std::to_string(gen);

	std::vector<uint8_t> dummy(640 * 480 * 3, 10);
	auto d0 = writer.write_frame(dummy.data(), dummy.size());
	EXPECT_EQ(d0.slot, 0u);
	EXPECT_EQ(d0.seq, 2u);

	auto d1 = writer.write_frame(dummy.data(), dummy.size());
	EXPECT_EQ(d1.slot, 1u);
	EXPECT_EQ(d1.seq, 4u);

	auto d2 = writer.write_frame(dummy.data(), dummy.size());
	EXPECT_EQ(d2.slot, 2u);
	EXPECT_EQ(d2.seq, 6u);

	// Overwrite slot 0
	auto d3 = writer.write_frame(dummy.data(), dummy.size());
	EXPECT_EQ(d3.slot, 0u);
	EXPECT_EQ(d3.seq, 8u);

	// Open file and verify slot 0 now has seq=8, not 2
	int fd = open(pool_path.c_str(), O_RDONLY);
	ASSERT_GE(fd, 0);
	auto *base = static_cast<uint8_t *>(mmap(nullptr, writer.get_total_size(), PROT_READ, MAP_SHARED, fd, 0));
	ASSERT_NE(base, MAP_FAILED);

	auto *slot0 = reinterpret_cast<FrameSlotHeader *>(base + 64);
	EXPECT_EQ(slot0->seq, 8u);

	munmap(base, writer.get_total_size());
	close(fd);
}
