#pragma once

#include <vector>
#include <string>
#include "mavlink_stream.h"

class Mavlink;

struct StreamListItem {
	MavlinkStream *(*new_instance)(Mavlink *mavlink);
	const char *(*get_name)();
	uint16_t (*get_id)();
};

const std::vector<StreamListItem> &get_streams_list();
MavlinkStream *create_mavlink_stream(const std::string &name, Mavlink *mavlink);
