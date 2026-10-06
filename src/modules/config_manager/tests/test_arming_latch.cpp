#include <gtest/gtest.h>
#include "arming_latch.h"

using cc::ArmingLatch;
using cc_msgs::msg::VehicleStatus;

TEST(ArmingLatchTest, BootStateAllowsApply)
{
	ArmingLatch latch;
	EXPECT_FALSE(latch.seen_heartbeat());
	EXPECT_FALSE(latch.is_armed());
	EXPECT_TRUE(latch.can_apply_links());
}

TEST(ArmingLatchTest, ArmedBlocksApply)
{
	ArmingLatch latch;
	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_ARMED);

	EXPECT_TRUE(latch.seen_heartbeat());
	EXPECT_TRUE(latch.is_armed());
	EXPECT_FALSE(latch.can_apply_links());
}

TEST(ArmingLatchTest, HeartbeatLossRetainsArmed)
{
	ArmingLatch latch;
	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_ARMED);
	EXPECT_FALSE(latch.can_apply_links());

	// Suppose heartbeat stops and no new updates arrive
	EXPECT_TRUE(latch.seen_heartbeat());
	EXPECT_TRUE(latch.is_armed());
	EXPECT_FALSE(latch.can_apply_links());
}

TEST(ArmingLatchTest, DisarmedAllowsApply)
{
	ArmingLatch latch;
	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_ARMED);
	EXPECT_FALSE(latch.can_apply_links());

	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_DISARMED);
	EXPECT_TRUE(latch.seen_heartbeat());
	EXPECT_FALSE(latch.is_armed());
	EXPECT_TRUE(latch.can_apply_links());
}

TEST(ArmingLatchTest, TransitionCycle)
{
	ArmingLatch latch;
	EXPECT_TRUE(latch.can_apply_links());

	// Disarmed initially
	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_DISARMED);
	EXPECT_TRUE(latch.can_apply_links());

	// Arm
	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_ARMED);
	EXPECT_FALSE(latch.can_apply_links());

	// Heartbeat drops (no change) -> still blocked
	EXPECT_FALSE(latch.can_apply_links());

	// Disarm
	latch.update_vehicle_status(VehicleStatus::ARMING_STATE_DISARMED);
	EXPECT_TRUE(latch.can_apply_links());
}
