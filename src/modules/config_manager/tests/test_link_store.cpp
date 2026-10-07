#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "link_store.h"

namespace fs = std::filesystem;
using cc::LinkStore;
using cc::LinkStorePaths;
using cc::LinkConfig;
using cc::Links;
using cc::ApplyResult;

class LinkStoreTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		_test_dir = fs::temp_directory_path() / ("test_link_store_" + std::to_string(std::rand()));
		fs::create_directories(_test_dir);
		_paths.active_file = (_test_dir / "active_links.json").string();
		_paths.default_file = (_test_dir / "default_links.json").string();
	}

	void TearDown() override
	{
		std::error_code ec;
		fs::remove_all(_test_dir, ec);
	}

	fs::path _test_dir;
	LinkStorePaths _paths;
};

TEST_F(LinkStoreTest, BootSeedsActiveFromDefault)
{
	// Write default links file
	Links def = {
		{"FC", "/dev/ttyAMA4", 921600},
		{"SIYI", "/dev/ttyAMA0", 115200}
	};
	ASSERT_TRUE(cc::save_links_file(_paths.default_file, def));

	LinkStore store(_paths, [](const std::string &) { return true; });
	auto act = store.active();
	ASSERT_EQ(act.size(), 2u);
	EXPECT_EQ(act[0].name, "FC");
	EXPECT_EQ(act[1].name, "SIYI");

	// Active file should have been written
	EXPECT_TRUE(fs::exists(_paths.active_file));
}

TEST_F(LinkStoreTest, ApplyWritesActiveAndCallsRouter)
{
	LinkStore store(_paths, [](const std::string &) { return true; });

	bool router_called = false;
	store.set_router_apply_fn([&](const Links & links, std::string &) {
		router_called = true;
		EXPECT_EQ(links.size(), 1u);
		return true;
	});

	Links new_links = {{"FC", "/dev/ttyUSB0", 115200}};
	std::string err;
	auto res = store.apply(new_links, err);

	EXPECT_EQ(res, ApplyResult::Ok);
	EXPECT_TRUE(router_called);
	EXPECT_EQ(store.active().size(), 1u);
	EXPECT_EQ(store.active()[0].port, "/dev/ttyUSB0");
}

TEST_F(LinkStoreTest, RouterFailureRollsBack)
{
	Links init = {{"FC", "/dev/ttyAMA4", 921600}};
	ASSERT_TRUE(cc::save_links_file(_paths.active_file, init));

	LinkStore store(_paths, [](const std::string &) { return true; });
	EXPECT_EQ(store.active()[0].baud, 921600u);

	store.set_router_apply_fn([](const Links &, std::string & err) {
		err = "router timeout";
		return false;
	});

	Links bad = {{"FC", "/dev/ttyAMA4", 57600}};
	std::string err;
	auto res = store.apply(bad, err);

	EXPECT_EQ(res, ApplyResult::Failed);
	EXPECT_EQ(err, "router timeout");

	// Active should remain old links
	EXPECT_EQ(store.active()[0].baud, 921600u);

	// File on disk should have rolled back to old links
	auto on_disk = cc::load_links_file(_paths.active_file);
	ASSERT_TRUE(on_disk.has_value());
	EXPECT_EQ((*on_disk)[0].baud, 921600u);
}

TEST_F(LinkStoreTest, PortValidationRejectsMissingPort)
{
	LinkStore store(_paths, [](const std::string & port) {
		return port == "/dev/ttyAMA4";
	});

	Links bad = {{"FC", "/dev/ttyUSB99", 115200}};
	std::string err;
	auto res = store.apply(bad, err);

	EXPECT_EQ(res, ApplyResult::Rejected);
	EXPECT_NE(err.find("port not found"), std::string::npos);
}

TEST_F(LinkStoreTest, SaveDefaultAndRestore)
{
	LinkStore store(_paths, [](const std::string &) { return true; }, [](const Links &, std::string &) { return true; });

	Links links1 = {{"FC", "/dev/ttyAMA4", 921600}};
	std::string err;
	EXPECT_EQ(store.apply(links1, err), ApplyResult::Ok);
	EXPECT_TRUE(store.save_default(err));

	// Change active to something else
	Links links2 = {{"FC", "/dev/ttyAMA4", 115200}};
	EXPECT_EQ(store.apply(links2, err), ApplyResult::Ok);
	EXPECT_EQ(store.active()[0].baud, 115200u);

	// Restore default
	EXPECT_EQ(store.restore_default(err), ApplyResult::Ok);
	EXPECT_EQ(store.active()[0].baud, 921600u);
}
