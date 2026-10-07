#pragma once

#include <cstdint>
#include <ctime>

using hrt_abstime = uint64_t;

/**
 * Get current monotonic time in microseconds (CLOCK_MONOTONIC).
 * Shared across all containers using the host kernel.
 */
inline hrt_abstime hrt_absolute_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<uint64_t>(ts.tv_sec) * 1000000ULL + static_cast<uint64_t>(ts.tv_nsec / 1000);
}

/**
 * Microseconds elapsed since 'then'.
 */
inline hrt_abstime hrt_elapsed_time(hrt_abstime then)
{
	return hrt_absolute_time() - then;
}
