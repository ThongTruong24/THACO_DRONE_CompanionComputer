#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "../schema_validator.h"

namespace fs = std::filesystem;

class SchemaValidatorTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		test_dir = fs::temp_directory_path() / ("test_schema_validator_" + std::to_string(std::time(nullptr)));
		fs::create_directories(test_dir);

		schema_file = test_dir / "param_schema.yaml";
		default_file = test_dir / "factory_default.yaml";
		user_file = test_dir / "cc_system.yaml";

		// Create a realistic test schema
		std::ofstream(schema_file) << R"(
version: "1.0"
parameters:
  router.fmu_baud:
    mav_id: "CC_RTR_BAUD"
    node: "config_manager"
    ros_name: "router.fmu_baud"
    type: int
    default: 921600
    options: [115200, 460800, 921600, 1500000]
    tier: 1
    desc: "Baudrate noi FMU"

  camera.fps:
    mav_id: "CC_CAM_FPS"
    node: "camera_streamer"
    ros_name: "fps"
    type: int
    default: 30
    min: 1
    max: 120
    tier: 2
    desc: "FPS camera"

  vision.enabled:
    mav_id: "CC_VIS_EN"
    node: "vision"
    ros_name: "enabled"
    type: bool
    default: true
    tier: 2
    desc: "YOLO enabled"

  vision.confidence:
    mav_id: "CC_VIS_CONF"
    node: "vision"
    ros_name: "confidence"
    type: float
    default: 0.35
    min: 0.05
    max: 0.95
    tier: 2
    desc: "Confidence"
)";

		// Create factory default
		std::ofstream(default_file) << R"(
version: "1.0"
drone_id: "THACO-DRONE-01"
router:
  fmu_baud: 921600
camera:
  fps: 30
vision:
  enabled: true
  confidence: 0.35
)";
	}

	void TearDown() override
	{
		fs::remove_all(test_dir);
	}

	fs::path test_dir;
	fs::path schema_file;
	fs::path default_file;
	fs::path user_file;
};

TEST_F(SchemaValidatorTest, LoadSchemaSuccess)
{
	cc::SchemaValidator validator;
	ASSERT_TRUE(validator.load_schema(schema_file.string()));
	EXPECT_TRUE(validator.is_schema_loaded());
	EXPECT_EQ(validator.get_params().size(), 4);
}

TEST_F(SchemaValidatorTest, ValidateHealthyConfig)
{
	cc::SchemaValidator validator;
	ASSERT_TRUE(validator.load_schema(schema_file.string()));

	auto res = validator.validate_file(default_file.string(), default_file.string());
	EXPECT_TRUE(res.is_valid);
	EXPECT_FALSE(res.is_degraded);
	EXPECT_EQ(res.validated_config["router"]["fmu_baud"].as<int>(), 921600);
	EXPECT_EQ(res.validated_config["camera"]["fps"].as<int>(), 30);
	EXPECT_TRUE(res.validated_config["vision"]["enabled"].as<bool>());
}

TEST_F(SchemaValidatorTest, FallbackOnMissingUserFile)
{
	cc::SchemaValidator validator;
	ASSERT_TRUE(validator.load_schema(schema_file.string()));

	fs::path non_existent = test_dir / "does_not_exist.yaml";
	auto res = validator.validate_file(non_existent.string(), default_file.string());

	EXPECT_TRUE(res.is_valid);
	EXPECT_TRUE(res.is_degraded); // Degraded because it had to fallback
	EXPECT_EQ(res.validated_config["router"]["fmu_baud"].as<int>(), 921600);
	ASSERT_FALSE(res.warnings.empty());
}

TEST_F(SchemaValidatorTest, Tier1RevertOnInvalidOption)
{
	cc::SchemaValidator validator;
	ASSERT_TRUE(validator.load_schema(schema_file.string()));

	// Create user config with invalid baudrate (not in options: [115200, 460800, 921600, 1500000])
	std::ofstream(user_file) << R"(
version: "1.0"
router:
  fmu_baud: 999999
camera:
  fps: 30
)";

	auto res = validator.validate_file(user_file.string(), default_file.string());
	EXPECT_TRUE(res.is_valid);
	EXPECT_TRUE(res.is_degraded);
	// Must be reverted to safe default 921600
	EXPECT_EQ(res.validated_config["router"]["fmu_baud"].as<int>(), 921600);
	ASSERT_FALSE(res.errors.empty());
}

TEST_F(SchemaValidatorTest, Tier2ClampOutOfBounds)
{
	cc::SchemaValidator validator;
	ASSERT_TRUE(validator.load_schema(schema_file.string()));

	// Create user config with fps = 200 (max is 120)
	std::ofstream(user_file) << R"(
version: "1.0"
router:
  fmu_baud: 921600
camera:
  fps: 200
vision:
  confidence: 1.5
)";

	auto res = validator.validate_file(user_file.string(), default_file.string());
	EXPECT_TRUE(res.is_valid);
	EXPECT_TRUE(res.is_degraded);
	// Clamped to max
	EXPECT_EQ(res.validated_config["camera"]["fps"].as<int>(), 120);
	EXPECT_FLOAT_EQ(res.validated_config["vision"]["confidence"].as<float>(), 0.95f);
	ASSERT_FALSE(res.warnings.empty());
}

TEST_F(SchemaValidatorTest, AtomicSaveAndActiveCache)
{
	cc::SchemaValidator validator;
	ASSERT_TRUE(validator.load_schema(schema_file.string()));

	auto res = validator.validate_file(default_file.string(), default_file.string());
	fs::path save_target = test_dir / "saved_config.yaml";
	fs::path cache_target = test_dir / "active_params.json";

	EXPECT_TRUE(cc::SchemaValidator::save_config_atomic(save_target.string(), res.validated_config));
	EXPECT_TRUE(fs::exists(save_target));

	EXPECT_TRUE(cc::SchemaValidator::write_active_cache(cache_target.string(), res.validated_config));
	EXPECT_TRUE(fs::exists(cache_target));

	// Verify json cache content
	std::ifstream jf(cache_target);
	nlohmann::json j;
	jf >> j;
	EXPECT_EQ(j["router"]["fmu_baud"], 921600);
	EXPECT_EQ(j["camera"]["fps"], 30);
}
