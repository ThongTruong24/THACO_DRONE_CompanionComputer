#pragma once

#include <cstdint>
#include <cc_msgs/msg/vehicle_status.hpp>

namespace cc
{

/**
 * ArmingLatch
 *
 * Implements vehicle arming interlock per spec §5.8 and D15:
 * - Boot state: has not seen HEARTBEAT from FC since startup -> allows applying links.
 *   (Necessary so the operator can fix baud rate / port configuration to restore FC comms).
 * - Once HEARTBEAT is received:
 *   - ARMED (ARMING_STATE_ARMED) -> blocks applying/restoring links.
 *   - Lost HEARTBEAT (no new vehicle_status updates) -> latches and retains last known state
 *     (if previously armed, continues blocking).
 *   - DISARMED (ARMING_STATE_DISARMED) -> allows applying/restoring links.
 *
 * ponytail: known limitation: if edge-agent restarts mid-flight while the FC link is already down,
 * the latch resets to "unseen" (unlocked). If persistent mid-flight protection across edge-agent crashes
 * is strictly required in the future, the latched armed state can be recorded into /run/drone/armed_latch.
 */
class ArmingLatch
{
public:
	ArmingLatch() = default;

	void update_vehicle_status(uint8_t arming_state)
	{
		_seen_heartbeat = true;

		if (arming_state == cc_msgs::msg::VehicleStatus::ARMING_STATE_ARMED) {
			_is_armed = true;

		} else if (arming_state == cc_msgs::msg::VehicleStatus::ARMING_STATE_DISARMED) {
			_is_armed = false;
		}
	}

	bool can_apply_links() const
	{
		if (!_seen_heartbeat) {
			return true;
		}

		return !_is_armed;
	}

	bool seen_heartbeat() const { return _seen_heartbeat; }
	bool is_armed() const { return _is_armed; }

	void reset()
	{
		_seen_heartbeat = false;
		_is_armed = false;
	}

private:
	bool _seen_heartbeat{false};
	bool _is_armed{false};
};

} // namespace cc
