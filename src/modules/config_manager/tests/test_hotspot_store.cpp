#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "hotspot_store.h"

namespace fs = std::filesystem;
using cc::HotspotStore;
using cc::HotspotStorePaths;

class HotspotStoreTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		_test_dir = fs::temp_directory_path() / ("test_hotspot_store_" + std::to_string(std::rand()));
		fs::create_directories(_test_dir);
		_paths.active_file = (_test_dir / "hotspot.json").string();
		_paths.wifi_file = (_test_dir / "wifi.json").string();
	}

	void TearDown() override
	{
		std::error_code ec;
		fs::remove_all(_test_dir, ec);
	}

	fs::path _test_dir;
	HotspotStorePaths _paths;
};

TEST_F(HotspotStoreTest, EffectiveFallbackToWifiFile)
{
	// Write wifi.json with hotspot
	nlohmann::json wifi = {
		{
			"hotspot", {
				{"ssid", "THACO_DRONE"},
				{"password", "thacodrone123"}
			}
		}
	};
	std::ofstream out(_paths.wifi_file);
	out << wifi.dump(2);
	out.close();

	HotspotStore store(_paths);
	EXPECT_TRUE(store.effective_valid());
	EXPECT_EQ(store.effective_ssid(), "THACO_DRONE");
	EXPECT_EQ(store.effective_password(), "thacodrone123");
}

TEST_F(HotspotStoreTest, ApplyWritesActiveHotspot)
{
	HotspotStore store(_paths);
	std::string err;
	EXPECT_TRUE(store.apply("MY_NEW_SSID", "secret12345", err));

	EXPECT_TRUE(store.effective_valid());
	EXPECT_EQ(store.effective_ssid(), "MY_NEW_SSID");
	EXPECT_EQ(store.effective_password(), "secret12345");
	EXPECT_TRUE(fs::exists(_paths.active_file));
}

TEST_F(HotspotStoreTest, SaveDefaultWritesWifiConfig)
{
	HotspotStore store(_paths);
	std::string err;
	EXPECT_TRUE(store.apply("SAVED_SSID", "mypassword1", err));
	EXPECT_TRUE(store.save_default(err));

	EXPECT_TRUE(fs::exists(_paths.wifi_file));

	std::ifstream in(_paths.wifi_file);
	auto j = nlohmann::json::parse(in);
	EXPECT_EQ(j["hotspot"]["ssid"], "SAVED_SSID");
	EXPECT_EQ(j["hotspot"]["password"], "mypassword1");
}

TEST_F(HotspotStoreTest, RestoreDefaultRemovesActive)
{
	// Default in wifi.json
	nlohmann::json wifi = {
		{
			"hotspot", {
				{"ssid", "DEFAULT_SSID"},
				{"password", "defaultpass1"}
			}
		}
	};
	std::ofstream out(_paths.wifi_file);
	out << wifi.dump(2);
	out.close();

	HotspotStore store(_paths);
	std::string err;
	EXPECT_TRUE(store.apply("TEMPORARY_SSID", "temppass123", err));
	EXPECT_EQ(store.effective_ssid(), "TEMPORARY_SSID");

	EXPECT_TRUE(store.restore_default(err));
	EXPECT_FALSE(fs::exists(_paths.active_file));
	EXPECT_EQ(store.effective_ssid(), "DEFAULT_SSID");
}

TEST_F(HotspotStoreTest, ValidationRules)
{
	HotspotStore store(_paths);
	std::string err;

	// Empty SSID
	EXPECT_FALSE(store.apply("", "validpassword", err));
	EXPECT_EQ(err, "invalid SSID");

	// Password too short (< 8)
	EXPECT_FALSE(store.apply("valid_ssid", "short", err));
	EXPECT_NE(err.find("8..63"), std::string::npos);

	// Password too long (> 63)
	std::string long_pw(64, 'a');
	EXPECT_FALSE(store.apply("valid_ssid", long_pw, err));
	EXPECT_NE(err.find("8..63"), std::string::npos);
}
