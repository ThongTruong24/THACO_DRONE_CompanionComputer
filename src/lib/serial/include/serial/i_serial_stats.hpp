#pragma once

#include <cstdint>
#include <string>
#include "hardware_registry.hpp"

namespace cc
{

/// Interface over kernel UART stats — mockable for testing without real hardware.
class ISerialStats
{
public:
	virtual ~ISerialStats() = default;
	virtual UartHardwareStats uart_stats(const std::string &device) = 0;
	virtual uint32_t          line_baud(const std::string &device) = 0;
};

} // namespace cc
