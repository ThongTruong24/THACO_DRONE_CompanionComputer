#pragma once

#include <cstdint>

class Mavlink;

class MavlinkStream
{
public:
	explicit MavlinkStream(Mavlink *mavlink);
	virtual ~MavlinkStream() = default;

	virtual const char *get_name() const = 0;
	virtual uint16_t get_id() = 0;
	virtual unsigned get_size() = 0;
	virtual bool send() = 0;

	void set_interval(uint32_t interval_us) { _interval_us = interval_us; }
	uint32_t get_interval() const { return _interval_us; }
	uint64_t get_last_sent() const { return _last_sent; }
	void set_last_sent(uint64_t t) { _last_sent = t; _first_run = false; }

	void update(uint64_t t);

protected:
	Mavlink *_mavlink{nullptr};
	uint32_t _interval_us{1000000};
	uint64_t _last_sent{0};
	bool _first_run{true};
};
