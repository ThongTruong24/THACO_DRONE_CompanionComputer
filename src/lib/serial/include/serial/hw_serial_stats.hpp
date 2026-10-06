#pragma once

#include <memory>
#include "i_serial_stats.hpp"
#include "hardware_registry.hpp"

namespace cc
{

/// ISerialStats backed by the real HardwareRegistry.
class HwSerialStats : public ISerialStats
{
public:
	explicit HwSerialStats(std::shared_ptr<HardwareRegistry> hw) : hw_(std::move(hw)) {}

	UartHardwareStats uart_stats(const std::string &device) override
	{
		return hw_->get_uart_stats(device);
	}

	uint32_t line_baud(const std::string &device) override
	{
		return hw_->get_uart_line_baud(device);
	}

private:
	std::shared_ptr<HardwareRegistry> hw_;
};

} // namespace cc
