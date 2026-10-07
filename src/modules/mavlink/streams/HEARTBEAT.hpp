#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"

class MavlinkStreamHeartbeat : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamHeartbeat(mavlink); }
	static constexpr const char *get_name_static() { return "HEARTBEAT"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_HEARTBEAT; }

	explicit MavlinkStreamHeartbeat(Mavlink *mavlink)
		: MavlinkStream(mavlink)
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_HEARTBEAT_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		mavlink_heartbeat_t hb{};
		hb.type = MAV_TYPE_ONBOARD_CONTROLLER;
		hb.autopilot = MAV_AUTOPILOT_INVALID;
		hb.base_mode = 0;
		hb.custom_mode = 0;
		hb.system_status = MAV_STATE_ACTIVE;
		mavlink_msg_heartbeat_send_struct(_mavlink->get_channel(), &hb);
		return true;
	}
};
