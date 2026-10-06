#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "mavlink_main.h"
#include "mavlink_parameters.h"
#include "mavlink_bridge_header.h"

class MavlinkParametersTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		if (!rclcpp::ok()) {
			rclcpp::init(0, nullptr);
		}
	}

	void SetUp() override
	{
		static int port_offset = 0;
		int port = 27600 + (port_offset++ * 2);
		mavlink_node = std::make_shared<Mavlink>(
				       "test_mavlink_param_" + std::to_string(port),
				       "127.0.0.1", port, port + 1);
		pm = mavlink_node->get_parameters_manager();
	}

	void TearDown() override
	{
		mavlink_node.reset();
	}

	std::shared_ptr<Mavlink> mavlink_node;
	MavlinkParametersManager *pm{nullptr};
};

TEST_F(MavlinkParametersTest, ParametersListIsSortedById)
{
	const auto &params = pm->all_params();
	ASSERT_FALSE(params.empty());

	for (size_t i = 0; i + 1 < params.size(); ++i) {
		EXPECT_LT(params[i].id, params[i + 1].id)
				<< "Params must be strictly sorted alphabetically by ID: "
				<< params[i].id << " vs " << params[i + 1].id;
	}
}

TEST_F(MavlinkParametersTest, AbsentNodeParametersAreOmitted)
{
	// All nodes ready initially
	pm->set_node_ready_checker([](const std::string &) { return true; });
	size_t full_count = pm->count();
	EXPECT_EQ(full_count, pm->all_params().size());

	// Now vision node is absent
	pm->set_node_ready_checker([](const std::string & name) {
		return name != "vision";
	});

	size_t filtered_count = pm->count();
	// Exactly 3 vision params (CC_VIS_CONF, CC_VIS_MODEL, CC_VIS_SRC) should be omitted
	EXPECT_EQ(filtered_count, full_count - 3);

	for (size_t i = 0; i < filtered_count; ++i) {
		ParamEntry entry;
		ASSERT_TRUE(pm->at(i, entry));
		EXPECT_NE(entry.node_name, "vision");
	}

	// Setting param of absent node must fail
	EXPECT_FALSE(pm->set("CC_VIS_MODEL", ParamValue::Str("new_model.pt")));
}

TEST_F(MavlinkParametersTest, SetSuccessAcceptedAndFailureRejectedWithTrueValue)
{
	pm->set_node_ready_checker([](const std::string &) { return true; });

	// Set initial value for CC_CAM_BR
	ParamValue initial_val = ParamValue::U32(2500);
	pm->set("CC_CAM_BR", initial_val);

	// 1. Successful set
	pm->set_param_setter([](const std::string &, const std::string &, const ParamValue &) {
		return true;
	});
	EXPECT_TRUE(pm->set("CC_CAM_BR", ParamValue::U32(4000)));
	ParamValue cur;
	EXPECT_TRUE(pm->get("CC_CAM_BR", cur));
	EXPECT_EQ(cur.u32, 4000u);

	// 2. Failed set (validation failed in node)
	pm->set_param_setter([](const std::string &, const std::string &, const ParamValue &) {
		return false; // rejected
	});
	EXPECT_FALSE(pm->set("CC_CAM_BR", ParamValue::U32(999999)));

	// Stored value remains the true current value (4000)
	EXPECT_TRUE(pm->get("CC_CAM_BR", cur));
	EXPECT_EQ(cur.u32, 4000u);
}

TEST_F(MavlinkParametersTest, BoolMappedToUint8)
{
	pm->set_node_ready_checker([](const std::string &) { return true; });
	pm->set_param_setter([](const std::string &, const std::string &, const ParamValue &) {
		return true;
	});

	ParamValue fpv;
	EXPECT_TRUE(pm->get("CC_CAM_FPV_EN", fpv));
	EXPECT_EQ(fpv.type, ParamType::UINT8);
	EXPECT_EQ(fpv.u8, 1u); // Default true

	// Set to false (0)
	EXPECT_TRUE(pm->set("CC_CAM_FPV_EN", ParamValue::U8(0)));
	EXPECT_TRUE(pm->get("CC_CAM_FPV_EN", fpv));
	EXPECT_EQ(fpv.u8, 0u);

	// Set to true (1)
	EXPECT_TRUE(pm->set("CC_CAM_FPV_EN", ParamValue::U8(1)));
	EXPECT_TRUE(pm->get("CC_CAM_FPV_EN", fpv));
	EXPECT_EQ(fpv.u8, 1u);
}

TEST_F(MavlinkParametersTest, LoadSchemaAndVerify16CharConstraint)
{
	// Test loading a dynamic schema file
	std::string candidate = "Companion_Computer/src/modules/config_manager/config/param_schema.yaml";
	if (!std::filesystem::exists(candidate)) {
		candidate = "../config_manager/config/param_schema.yaml";
	}

	if (std::filesystem::exists(candidate)) {
		EXPECT_TRUE(pm->load_schema(candidate));
		const auto &params = pm->all_params();
		EXPECT_GT(params.size(), 10u);

		for (const auto &p : params) {
			EXPECT_LE(p.id.size(), 16u) << "Param ID " << p.id << " exceeds 16 chars";
			EXPECT_FALSE(p.node_name.empty());
			EXPECT_FALSE(p.ros_name.empty());
		}

		for (size_t i = 0; i + 1 < params.size(); ++i) {
			EXPECT_LT(params[i].id, params[i + 1].id)
					<< "Params must be strictly sorted alphabetically by ID: "
					<< params[i].id << " vs " << params[i + 1].id;
		}
	}

	// Loading non-existent file should gracefully return false
	EXPECT_FALSE(pm->load_schema("/tmp/non_existent_schema_12345.yaml"));
}
