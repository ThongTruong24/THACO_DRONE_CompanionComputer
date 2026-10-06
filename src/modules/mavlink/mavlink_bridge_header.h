#pragma once

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS
#define MAVLINK_SEND_UART_BYTES mavlink_send_uart_bytes

#include <cstdint>
#include <unistd.h>
#include <mavlink_types.h>

class Mavlink;

extern mavlink_system_t mavlink_system;
void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t *ch, int length);

#include <thaco/mavlink.h>
