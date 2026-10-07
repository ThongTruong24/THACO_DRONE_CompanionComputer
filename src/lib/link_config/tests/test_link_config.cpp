#include <gtest/gtest.h>
#include <filesystem>
#include <unistd.h>
#include "link_config.hpp"

using namespace cc;

static const char *kTwo = R"({"links":[{"name":"FC","port":"/dev/ttyAMA4","baud":921600},
                                      {"name":"SIYI","port":"/dev/ttyAMA0","baud":115200}]})";

TEST(LinkConfig, ParsesValidList)
{
	auto l = parse_links_json(kTwo);
	ASSERT_TRUE(l);
	ASSERT_EQ(l->size(), 2u);
	EXPECT_EQ((*l)[1], (LinkConfig{"SIYI", "/dev/ttyAMA0", 115200}));
}

TEST(LinkConfig, RejectsInjectionAndBadValues)
{
	EXPECT_FALSE(parse_links_json(R"({"links":[{"name":"A\nB","port":"/dev/ttyAMA4","baud":921600}]})"));
	EXPECT_FALSE(parse_links_json(R"({"links":[{"name":"A","port":"/dev/ttyAMA4\nX=1","baud":921600}]})"));
	EXPECT_FALSE(parse_links_json(R"({"links":[{"name":"A","port":"/etc/passwd","baud":921600}]})"));
	EXPECT_FALSE(parse_links_json(R"({"links":[{"name":"A","port":"/dev/ttyAMA4","baud":12345}]})"));
	EXPECT_FALSE(parse_links_json(R"({"links":[{"name":"A","port":"/dev/ttyAMA4","baud":921600},
                                               {"name":"B","port":"/dev/ttyAMA4","baud":115200}]})"));
	EXPECT_FALSE(parse_links_json(R"({"links":[]})"));
	EXPECT_FALSE(parse_links_json("not json"));
}

TEST(LinkConfig, AtomicSaveRoundTrip)
{
	auto path = (std::filesystem::temp_directory_path() / ("links_" + std::to_string(getpid()) + ".json")).string();
	auto l = *parse_links_json(kTwo);
	ASSERT_TRUE(save_links_file(path, l));
	EXPECT_EQ(load_links_file(path), l);
	EXPECT_FALSE(std::filesystem::exists(path + ".tmp"));
	std::filesystem::remove(path);
}
