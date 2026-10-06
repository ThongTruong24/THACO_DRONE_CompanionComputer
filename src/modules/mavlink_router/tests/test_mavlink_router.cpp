#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "mavlink_router/mavlink_router.hpp"

namespace fs = std::filesystem;
using namespace cc;

class MavlinkRouterTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		test_dir_ = fs::temp_directory_path() / ("mlr_test_" + std::to_string(getpid()));
		fs::create_directories(test_dir_);

		mock_routerd_ = test_dir_ / "mock_routerd.sh";
		{
			std::ofstream out(mock_routerd_);
			out << "#!/bin/sh\n"
			    << "echo 'UART Endpoint [1] Radio {'\n"
			    << "echo '}'\n"
			    << "while true; do sleep 0.1; done\n";
		}
		chmod(mock_routerd_.c_str(), 0755);

		conf_path_ = test_dir_ / "mavlink-router.conf";
		active_links_ = test_dir_ / "active.json";
		default_links_ = test_dir_ / "default.json";

		{
			std::ofstream out(active_links_);
			out << R"({"links":[{"name":"L1","port":"/dev/ttyUSB0","baud":115200}]})";
		}
		{
			std::ofstream out(default_links_);
			out << R"({"links":[{"name":"L1","port":"/dev/ttyUSB0","baud":57600}]})";
		}
	}

	void TearDown() override
	{
		fs::remove_all(test_dir_);
	}

	fs::path test_dir_;
	fs::path mock_routerd_;
	fs::path conf_path_;
	fs::path active_links_;
	fs::path default_links_;
};

TEST_F(MavlinkRouterTest, StartsWithoutRosAndParsesStats)
{
	MavlinkRouter router(mock_routerd_.string(), conf_path_.string());
	ASSERT_TRUE(router.start(active_links_.string(), default_links_.string()));
	EXPECT_TRUE(router.is_running());

	// Wait briefly for child to output
	std::this_thread::sleep_for(std::chrono::milliseconds(150));
	EXPECT_TRUE(router.is_child_alive());

	auto cfg = router.get_current_config();
	ASSERT_EQ(cfg.links.size(), 1u);
	EXPECT_EQ(cfg.links[0].name, "L1");
	EXPECT_EQ(cfg.links[0].baud, 115200u);

	router.stop();
	EXPECT_FALSE(router.is_running());
	EXPECT_FALSE(router.is_child_alive());
}

TEST_F(MavlinkRouterTest, ActiveWinsOverDefault)
{
	MavlinkRouter router(mock_routerd_.string(), conf_path_.string());
	ASSERT_TRUE(router.start(active_links_.string(), default_links_.string()));
	EXPECT_EQ(router.get_current_config().links.at(0).baud, 115200u);
	router.stop();
}

TEST_F(MavlinkRouterTest, DefaultWinsWhenActiveMissing)
{
	fs::remove(active_links_);
	MavlinkRouter router(mock_routerd_.string(), conf_path_.string());
	ASSERT_TRUE(router.start(active_links_.string(), default_links_.string()));
	EXPECT_EQ(router.get_current_config().links.at(0).baud, 57600u);
	router.stop();
}

TEST_F(MavlinkRouterTest, ApplyLinksRestartsChild)
{
	MavlinkRouter router(mock_routerd_.string(), conf_path_.string());
	ASSERT_TRUE(router.start(active_links_.string(), default_links_.string()));

	Links new_links = {{"NewLink", "/dev/ttyUSB1", 921600}};
	std::string err;
	EXPECT_TRUE(router.apply_links(new_links, err)) << err;

	EXPECT_EQ(router.get_current_config().links.at(0).name, "NewLink");
	EXPECT_EQ(router.get_current_config().links.at(0).baud, 921600u);

	router.stop();
}

TEST_F(MavlinkRouterTest, ApplyLinksRejectsWhenStopped)
{
	// CDO.3 fix verification
	MavlinkRouter router(mock_routerd_.string(), conf_path_.string());
	ASSERT_TRUE(router.start(active_links_.string(), default_links_.string()));
	router.stop();

	Links new_links = {{"L1", "/dev/ttyUSB0", 921600}};
	std::string err;
	EXPECT_FALSE(router.apply_links(new_links, err));
	EXPECT_FALSE(router.is_child_alive());
}

TEST_F(MavlinkRouterTest, StopInterruptsPromptly)
{
	// CDO.2 fix verification: hotplug condvar wait is interrupted by stop()
	MavlinkRouter router(mock_routerd_.string(), conf_path_.string());
	ASSERT_TRUE(router.start(active_links_.string(), default_links_.string()));

	auto t0 = std::chrono::steady_clock::now();
	router.stop();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			       std::chrono::steady_clock::now() - t0).count();

	// Must stop well under 2 seconds, not blocked by 5 second hotplug sleep
	EXPECT_LT(elapsed, 1500);
}
