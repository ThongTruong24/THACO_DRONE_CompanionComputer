#include "mavlink_messages.h"
#include "streams/HEARTBEAT.hpp"
#include "streams/STATUSTEXT.hpp"
#include "streams/CC_SERIAL_LINK.hpp"
#include "streams/CC_TELEMETRY_LINKS.hpp"
#include "streams/CC_TELEMETRY_CAMERA.hpp"
#include "streams/CC_TELEMETRY_NETWORK.hpp"
#include "streams/CC_TELEMETRY_VISION.hpp"
#include "streams/CC_TELEMETRY_SYSTEM.hpp"

namespace
{
template <typename T>
StreamListItem create_stream_list_item()
{
	return StreamListItem{
		&T::new_instance,
		&T::get_name_static,
		&T::get_id_static
	};
}

const std::vector<StreamListItem> streams_list = {
	create_stream_list_item<MavlinkStreamHeartbeat>(),
	create_stream_list_item<MavlinkStreamStatustext>(),
	create_stream_list_item<MavlinkStreamCcSerialLink>(),
	create_stream_list_item<MavlinkStreamCcTelemetryLinks>(),
	create_stream_list_item<MavlinkStreamCcTelemetryCamera>(),
	create_stream_list_item<MavlinkStreamCcTelemetryNetwork>(),
	create_stream_list_item<MavlinkStreamCcTelemetryVision>(),
	create_stream_list_item<MavlinkStreamCcTelemetrySystem>()
};
} // namespace

const std::vector<StreamListItem> &get_streams_list()
{
	return streams_list;
}

MavlinkStream *create_mavlink_stream(const std::string &name, Mavlink *mavlink)
{
	for (const auto &item : streams_list) {
		if (name == item.get_name()) {
			return item.new_instance(mavlink);
		}
	}

	return nullptr;
}
