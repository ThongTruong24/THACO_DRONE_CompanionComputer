#include "mavlink_stream.h"

MavlinkStream::MavlinkStream(Mavlink *mavlink)
	: _mavlink(mavlink)
{
}

void MavlinkStream::update(uint64_t t)
{
	if (_first_run || t >= _last_sent + _interval_us) {
		if (send()) {
			_last_sent = t;
			_first_run = false;
		}
	}
}
