#pragma once

#include <cstdarg>
#include <cstdio>
#include <string>
#include <type_traits>
#include <rclcpp/rclcpp.hpp>
#include <cc_msgs/msg/mavlink_log.hpp>
#include <hrt/hrt.hpp>

#define MAVLINK_LOG_MAXLEN 127

#define _MSG_PRIO_DEBUG     7
#define _MSG_PRIO_INFO      6
#define _MSG_PRIO_NOTICE    5
#define _MSG_PRIO_WARNING   4
#define _MSG_PRIO_ERROR     3
#define _MSG_PRIO_CRITICAL  2
#define _MSG_PRIO_ALERT     1
#define _MSG_PRIO_EMERGENCY 0

template <typename PubT>
inline void mavlink_log_publish(PubT &pub, uint8_t severity, const char *fmt, ...)
{
	char buf[256];
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	if (n < 0) { return; }

	cc_msgs::msg::MavlinkLog msg;
	msg.timestamp = hrt_absolute_time();
	msg.severity = severity;
	std::string text(buf);

	if (text.size() > MAVLINK_LOG_MAXLEN) {
		text.resize(MAVLINK_LOG_MAXLEN);
	}

	msg.text = text;

	if constexpr(std::is_pointer_v<std::decay_t<PubT>>) {
		if (pub) {
			pub->publish(msg);
		}

	} else {
		if (pub) {
			pub->publish(msg);
		}
	}
}

#define mavlink_log_info(_pub, _text, ...) \
	mavlink_log_publish(_pub, _MSG_PRIO_INFO, _text, ##__VA_ARGS__)

#define mavlink_log_warning(_pub, _text, ...) \
	mavlink_log_publish(_pub, _MSG_PRIO_WARNING, _text, ##__VA_ARGS__)

#define mavlink_log_critical(_pub, _text, ...) \
	mavlink_log_publish(_pub, _MSG_PRIO_CRITICAL, _text, ##__VA_ARGS__)

#define mavlink_log_emergency(_pub, _text, ...) \
	mavlink_log_publish(_pub, _MSG_PRIO_EMERGENCY, _text, ##__VA_ARGS__)
