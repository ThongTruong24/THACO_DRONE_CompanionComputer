#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <serial/link_stats.hpp>

namespace cc
{

class RouterStatsParser
{
public:
	RouterStatsParser() = default;

	void feed_line(const std::string &line);

	std::map<std::string, RouterEndpointStat> get_endpoint_stats() const;

	void reset();

private:
	struct EndpointInternal {
		std::string name;
		std::string type;
		uint32_t rx_handled{0};
		uint32_t rx_bytes{0};
		uint32_t rx_crc_errors{0};
		uint32_t rx_seq_lost{0};
		float rx_loss_pct{0.0f};
		uint32_t tx_total{0};
		uint32_t tx_bytes{0};
		float rx_rate{0.0f};
		float tx_rate{0.0f};
		bool online{false};

		uint32_t prev_rx_handled{0};
		uint32_t prev_rx_bytes{0};
		uint32_t prev_tx_bytes{0};
		std::chrono::steady_clock::time_point last_stat_time{};
		std::chrono::steady_clock::time_point last_rx_time{};
	};

	mutable std::mutex mutex_;
	std::string current_endpoint_;
	std::string current_section_;
	std::map<std::string, EndpointInternal> endpoints_;
};

} // namespace cc
