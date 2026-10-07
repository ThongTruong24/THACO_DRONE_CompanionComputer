#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "hardware_registry.hpp"

namespace cc
{

constexpr uint8_t LINK_STATUS_DISCONNECTED = 0;
constexpr uint8_t LINK_STATUS_CONNECTED    = 2;

struct GenericLinkSnapshot {
	std::string name;
	std::string port;
	uint32_t baud{0};
	float rx_rate{0.0f};
	float tx_rate{0.0f};
	float rx_loss{0.0f};
	uint32_t rx_bytes{0};
	uint32_t tx_bytes{0};
	uint32_t rx_errors{0};
	uint8_t status{LINK_STATUS_DISCONNECTED};
};

struct RouterEndpointStat {
	float rx_rate{0.0f};
	float tx_rate{0.0f};
	uint32_t rx_bytes{0};
	uint32_t tx_bytes{0};
	float rx_loss_pct{0.0f};
	bool online{false};
};

struct RouterStatsSnapshot {
	bool valid{false};
	std::map<std::string, RouterEndpointStat> endpoints;
};

struct LinkStatsFallback {
	float    fc_mavlink_rx_rate  = 0.0f;
	uint32_t fc_mavlink_rx_total = 0;
	uint32_t self_tx_total       = 0;
	bool     fc_online_hint      = false;
	uint32_t fc_baud             = 0;
	uint32_t siyi_baud           = 0;
	uint8_t  transport_type      = 1;

	bool     router_stats_valid  = false;
	float    fc_router_rx_rate   = 0.0f;
	float    fc_router_tx_rate   = 0.0f;
	uint32_t fc_router_rx_bytes  = 0;
	uint32_t fc_router_tx_bytes  = 0;
	float    fc_router_loss_pct  = 0.0f;
	bool     fc_router_online    = false;

	float    siyi_router_rx_rate  = 0.0f;
	float    siyi_router_tx_rate  = 0.0f;
	uint32_t siyi_router_rx_bytes = 0;
	uint32_t siyi_router_tx_bytes = 0;
	float    siyi_router_loss_pct = 0.0f;
	bool     siyi_router_online   = false;

	uint32_t siyi_mavlink_rx_total = 0;
	bool     siyi_mavlink_online   = false;
};

struct LinkFields {
	float    fc_tx_rate        = 0.0f;
	float    fc_rx_rate        = 0.0f;
	float    fc_tx_rate_max    = 0.0f;
	float    fc_tx_rate_multi  = 1.0f;
	float    fc_rx_loss        = 0.0f;
	uint32_t fc_tx_err         = 0;
	uint32_t fc_bytes_rx       = 0;
	uint32_t fc_bytes_tx       = 0;
	uint32_t fc_baudrate       = 0;
	float    siyi_tx_rate      = 0.0f;
	float    siyi_rx_rate      = 0.0f;
	float    siyi_tx_rate_max  = 0.0f;
	float    siyi_tx_rate_multi = 1.0f;
	float    siyi_rx_loss      = 0.0f;
	uint32_t siyi_tx_err       = 0;
	uint32_t siyi_bytes_rx     = 0;
	uint32_t siyi_bytes_tx     = 0;
	uint32_t siyi_baudrate     = 0;
	uint8_t  fc_status         = 0;
	uint8_t  siyi_status       = 0;
	uint8_t  transport_type    = 1;
};

class LinkStatsComputer
{
public:
	static constexpr int kHoldTicks = 3;

	// Generalized links computation (Task T8, PC Task 15)
	template <typename LinkConfigListT>
	std::vector<GenericLinkSnapshot> update_generic(
		const LinkConfigListT &links,
		const RouterStatsSnapshot &router,
		const std::function<UartHardwareStats(const std::string &)> &uart_fn)
	{
		std::vector<GenericLinkSnapshot> out;

		for (const auto &l : links) {
			GenericLinkSnapshot s;
			s.name = l.name;
			s.port = l.port;
			s.baud = l.baud;

			bool online = false;

			if (router.valid) {
				auto it = router.endpoints.find(l.name);

				if (it != router.endpoints.end()) {
					const auto &e = it->second;
					s.rx_rate = e.rx_rate;
					s.tx_rate = e.tx_rate;
					s.rx_loss = e.rx_loss_pct;
					s.rx_bytes = e.rx_bytes;
					s.tx_bytes = e.tx_bytes;
					online = e.online;
				}
			}

			if (uart_fn) {
				UartHardwareStats hw = uart_fn(l.port);

				if (hw.valid) {
					s.rx_errors = hw.rx_errors;

					if (!router.valid && hw.rx_bytes > 0) {
						online = true;
						s.rx_rate = static_cast<float>(hw.rx_bytes);
						s.tx_rate = static_cast<float>(hw.tx_bytes);
					}
				}
			}

			int &t = hold_[l.name];
			t = online ? kHoldTicks : (t > 0 ? t - 1 : 0);
			s.status = (t > 0) ? LINK_STATUS_CONNECTED : LINK_STATUS_DISCONNECTED;
			out.push_back(s);
		}

		return out;
	}

	// Legacy 2-link update (kept for compatibility with cc_agent_legacy)
	LinkFields update(const UartHardwareStats &fc_hw,
			  const UartHardwareStats &siyi_hw,
			  const LinkStatsFallback &fb)
	{
		LinkFields f;
		f.transport_type = fb.transport_type;
		f.fc_baudrate    = fb.fc_baud;
		f.siyi_baudrate  = fb.siyi_baud;

		// Flight Controller
		uint32_t fc_rx_delta = 0, fc_tx_delta = 0;
		bool fc_active = false;

		if (fb.router_stats_valid) {
			f.fc_rx_rate  = fb.fc_router_rx_rate;
			f.fc_tx_rate  = fb.fc_router_tx_rate;
			f.fc_bytes_rx = fb.fc_router_rx_bytes;
			f.fc_bytes_tx = fb.fc_router_tx_bytes;
			f.fc_rx_loss  = fb.fc_router_loss_pct;
			f.fc_tx_err   = 0;
			fc_active     = fb.fc_router_online || fb.fc_online_hint;

		} else if (fc_hw.valid && fc_hw.rx_bytes > 0) {
			fc_rx_delta   = fc_hw.rx_bytes;
			fc_tx_delta   = fc_hw.tx_bytes;
			f.fc_rx_rate  = static_cast<float>(fc_rx_delta);
			f.fc_tx_rate  = static_cast<float>(fc_tx_delta);
			fc_rx_total_ += fc_rx_delta;
			fc_tx_total_ += fc_tx_delta;
			f.fc_bytes_rx = fc_rx_total_;
			f.fc_bytes_tx = fc_tx_total_;
			bool is_noise = (fc_hw.frame_errors > 0 && fc_hw.frame_errors > (fc_hw.rx_bytes / 2));

			if (is_noise) {
				f.fc_rx_loss = 0.0f;
				fc_active = false;

			} else {
				f.fc_rx_loss = loss_pct(fc_hw.frame_errors + fc_hw.overrun_errors, fc_hw.rx_bytes);
				fc_active = true;
			}

			f.fc_tx_err   = fc_hw.buf_overrun;

		} else {
			fc_rx_delta   = delta(fb.fc_mavlink_rx_total, last_fc_mav_rx_);
			fc_tx_delta   = delta(fb.self_tx_total, last_self_tx_);
			f.fc_rx_rate  = (fb.fc_mavlink_rx_rate > 0.0f) ? fb.fc_mavlink_rx_rate : static_cast<float>(fc_rx_delta);
			f.fc_tx_rate  = static_cast<float>(fc_tx_delta);
			f.fc_bytes_rx = fb.fc_mavlink_rx_total;
			f.fc_bytes_tx = fb.self_tx_total;
			f.fc_rx_loss  = 0.0f;
			f.fc_tx_err   = 0;
			fc_active     = fb.fc_online_hint;
		}

		last_fc_mav_rx_ = fb.fc_mavlink_rx_total;
		last_self_tx_   = fb.self_tx_total;

		fc_tx_peak_      = std::max(fc_tx_peak_, f.fc_tx_rate);
		f.fc_tx_rate_max = fc_tx_peak_;
		fc_active_ticks_ = tick_window(fc_active_ticks_, fc_active);
		f.fc_status      = (fc_active_ticks_ > 0) ? 2 : 0;

		// SIYI
		uint32_t siyi_rx_delta = 0, siyi_tx_delta = 0;
		bool siyi_active = false;

		if (fb.router_stats_valid) {
			f.siyi_rx_rate  = fb.siyi_router_rx_rate;
			f.siyi_tx_rate  = fb.siyi_router_tx_rate;
			f.siyi_bytes_rx = fb.siyi_router_rx_bytes;
			f.siyi_bytes_tx = fb.siyi_router_tx_bytes;
			f.siyi_rx_loss  = fb.siyi_router_loss_pct;
			f.siyi_tx_err   = 0;
			siyi_active     = fb.siyi_router_online || fb.siyi_mavlink_online;

		} else if (fb.siyi_mavlink_online || fb.siyi_mavlink_rx_total > 0) {
			uint32_t siyi_mav_delta = delta(fb.siyi_mavlink_rx_total, last_siyi_mav_rx_);
			f.siyi_rx_rate  = static_cast<float>(siyi_mav_delta);
			f.siyi_tx_rate  = 0.0f;
			f.siyi_bytes_rx = fb.siyi_mavlink_rx_total;
			f.siyi_bytes_tx = 0;
			f.siyi_rx_loss  = 0.0f;
			f.siyi_tx_err   = 0;
			siyi_active     = fb.siyi_mavlink_online;

		} else {
			if (siyi_hw.valid) {
				siyi_tx_delta  = siyi_hw.tx_bytes;
				f.siyi_tx_err  = siyi_hw.buf_overrun;
				f.siyi_rx_loss = loss_pct(siyi_hw.frame_errors + siyi_hw.overrun_errors, siyi_hw.rx_bytes);
				bool is_noise  = (siyi_hw.frame_errors > 0 && siyi_hw.frame_errors > (siyi_hw.rx_bytes / 2));

				if (is_noise) {
					siyi_rx_delta = 0;
					siyi_active   = false;

				} else {
					siyi_rx_delta = siyi_hw.rx_bytes;
					siyi_active   = (siyi_hw.rx_bytes > 0);
				}
			}

			f.siyi_rx_rate  = static_cast<float>(siyi_rx_delta);
			f.siyi_tx_rate  = static_cast<float>(siyi_tx_delta);
			siyi_rx_total_ += siyi_rx_delta;
			siyi_tx_total_ += siyi_tx_delta;
			f.siyi_bytes_rx = siyi_rx_total_;
			f.siyi_bytes_tx = siyi_tx_total_;
		}

		last_siyi_mav_rx_ = fb.siyi_mavlink_rx_total;

		siyi_tx_peak_      = std::max(siyi_tx_peak_, f.siyi_tx_rate);
		f.siyi_tx_rate_max = siyi_tx_peak_;
		siyi_active_ticks_ = tick_window(siyi_active_ticks_, siyi_active);
		f.siyi_status      = (siyi_active_ticks_ > 0) ? 2 : 0;

		return f;
	}

private:
	static int tick_window(int cur, bool active)
	{
		if (active) { return kHoldTicks; }

		return (cur > 0) ? cur - 1 : 0;
	}

	static float loss_pct(uint32_t err, uint32_t rx)
	{
		if (err == 0) { return 0.0f; }

		uint64_t denom = static_cast<uint64_t>(rx) + err;

		if (denom == 0) { return 0.0f; }

		return std::min(100.0f, 100.0f * static_cast<float>(err) / static_cast<float>(denom));
	}

	static uint32_t delta(uint32_t cur, uint32_t last) { return cur >= last ? cur - last : 0; }

	std::map<std::string, int> hold_;

	uint32_t fc_rx_total_ = 0, fc_tx_total_ = 0;
	uint32_t siyi_rx_total_ = 0, siyi_tx_total_ = 0;
	uint32_t last_fc_mav_rx_ = 0, last_self_tx_ = 0;
	uint32_t last_siyi_mav_rx_ = 0;
	float    fc_tx_peak_ = 0.0f, siyi_tx_peak_ = 0.0f;
	int      fc_active_ticks_ = 0, siyi_active_ticks_ = 0;
};

} // namespace cc
