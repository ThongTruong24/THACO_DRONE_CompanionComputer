#include "mavlink_router/router_stats_parser.hpp"
#include <cstdio>

namespace cc
{

void RouterStatsParser::reset()
{
	std::lock_guard<std::mutex> lock(mutex_);
	current_endpoint_.clear();
	current_section_.clear();
	endpoints_.clear();
}

void RouterStatsParser::feed_line(const std::string &line)
{
	std::lock_guard<std::mutex> lock(mutex_);

	// 1. Endpoint header: e.g. "UART Endpoint [3] Radio {" or "UDP Endpoint [5] QGC_UDP_Server {"
	size_t ep_pos = line.find("Endpoint [");

	if (ep_pos != std::string::npos) {
		size_t close_bracket = line.find(']', ep_pos);

		if (close_bracket != std::string::npos) {
			size_t open_brace = line.find('{', close_bracket);
			std::string ep_name;

			if (open_brace != std::string::npos) {
				ep_name = line.substr(close_bracket + 1, open_brace - close_bracket - 1);

			} else {
				ep_name = line.substr(close_bracket + 1);
			}

			size_t first = ep_name.find_first_not_of(" \t");
			size_t last = ep_name.find_last_not_of(" \t");

			if (first != std::string::npos && last != std::string::npos) {
				current_endpoint_ = ep_name.substr(first, last - first + 1);

			} else {
				current_endpoint_.clear();
			}

			current_section_.clear();

			if (!current_endpoint_.empty()) {
				auto &st = endpoints_[current_endpoint_];
				st.name = current_endpoint_;
				std::string prefix = line.substr(0, ep_pos);
				size_t p_first = prefix.find_first_not_of(" \t");
				size_t p_last = prefix.find_last_not_of(" \t");

				if (p_first != std::string::npos && p_last != std::string::npos) {
					st.type = prefix.substr(p_first, p_last - p_first + 1);
				}
			}

			return;
		}
	}

	if (current_endpoint_.empty()) {
		return;
	}

	if (line.find("Received messages {") != std::string::npos) {
		current_section_ = "RX";
		return;
	}

	if (line.find("Transmitted messages {") != std::string::npos) {
		current_section_ = "TX";
		return;
	}

	auto &st = endpoints_[current_endpoint_];

	if (current_section_ == "RX") {
		if (line.find("CRC error:") != std::string::npos) {
			unsigned int crc_err = 0, crc_pct = 0;
			unsigned long crc_kb = 0;
			const char *p = line.c_str() + line.find("CRC error:");

			if (sscanf(p, "CRC error: %u %u%% %luKB", &crc_err, &crc_pct, &crc_kb) >= 1) {
				st.rx_crc_errors = crc_err;
			}

		} else if (line.find("Sequence lost:") != std::string::npos) {
			unsigned int seq_lost = 0, seq_pct = 0;
			const char *p = line.c_str() + line.find("Sequence lost:");

			if (sscanf(p, "Sequence lost: %u %u%%", &seq_lost, &seq_pct) >= 2) {
				st.rx_seq_lost = seq_lost;
				st.rx_loss_pct = static_cast<float>(seq_pct);
			}

		} else if (line.find("Handled:") != std::string::npos) {
			unsigned int handled = 0;
			unsigned long handled_kb = 0;
			const char *p = line.c_str() + line.find("Handled:");

			if (sscanf(p, "Handled: %u %luKB", &handled, &handled_kb) >= 1) {
				st.rx_handled = handled;
				st.rx_bytes = handled_kb * 1000;
			}
		}

	} else if (current_section_ == "TX") {
		if (line.find("Total:") != std::string::npos) {
			unsigned int tx_total = 0;
			unsigned long tx_kb = 0;
			const char *p = line.c_str() + line.find("Total:");

			if (sscanf(p, "Total: %u %luKB", &tx_total, &tx_kb) >= 1) {
				st.tx_total = tx_total;
				st.tx_bytes = tx_kb * 1000;
			}
		}
	}

	if (line == "}" || line == "\t}" || line.find("}") != std::string::npos) {
		auto now = std::chrono::steady_clock::now();

		if (st.last_stat_time.time_since_epoch().count() == 0) {
			st.prev_rx_handled = st.rx_handled;
			st.prev_rx_bytes = st.rx_bytes;
			st.prev_tx_bytes = st.tx_bytes;
			st.last_stat_time = now;

			if (st.rx_handled > 0) {
				st.last_rx_time = now;
				st.online = true;
			}

		} else {
			double dt = std::chrono::duration<double>(now - st.last_stat_time).count();

			if (dt >= 0.5) {
				uint64_t d_rx = (st.rx_bytes >= st.prev_rx_bytes) ? (st.rx_bytes - st.prev_rx_bytes) : 0;
				uint64_t d_tx = (st.tx_bytes >= st.prev_tx_bytes) ? (st.tx_bytes - st.prev_tx_bytes) : 0;
				uint32_t d_handled = (st.rx_handled >= st.prev_rx_handled) ? (st.rx_handled - st.prev_rx_handled) : 0;

				if (d_handled > 0) {
					st.last_rx_time = now;

					if (d_rx == 0) {
						d_rx = d_handled * 40;
					}
				}

				st.rx_rate = static_cast<float>(d_rx / dt);
				st.tx_rate = static_cast<float>(d_tx / dt);
				st.prev_rx_bytes = st.rx_bytes;
				st.prev_tx_bytes = st.tx_bytes;
				st.prev_rx_handled = st.rx_handled;
				st.last_stat_time = now;
			}
		}

		if (st.last_rx_time.time_since_epoch().count() > 0) {
			double since_rx = std::chrono::duration<double>(now - st.last_rx_time).count();
			st.online = (st.rx_handled > 0 && since_rx <= 3.0);

		} else {
			st.online = false;
		}
	}
}

std::map<std::string, RouterEndpointStat> RouterStatsParser::get_endpoint_stats() const
{
	std::lock_guard<std::mutex> lock(mutex_);
	std::map<std::string, RouterEndpointStat> out;

	for (const auto &[name, st] : endpoints_) {
		RouterEndpointStat s;
		s.rx_rate = st.rx_rate;
		s.tx_rate = st.tx_rate;
		s.rx_bytes = st.rx_bytes;
		s.tx_bytes = st.tx_bytes;
		s.rx_loss_pct = st.rx_loss_pct;
		s.online = st.online;
		out[name] = s;
	}

	return out;
}

} // namespace cc
