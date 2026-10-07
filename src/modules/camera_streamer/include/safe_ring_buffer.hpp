#pragma once

#include "camera_interface.hpp"
#include <array>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace drone
{

class SafeRingBuffer
{
public:
	static constexpr size_t BUFFER_SIZE = 4;

	SafeRingBuffer() : m_write_idx(0), m_read_idx(0), m_count(0), m_dropped_count(0) {}

	// Đẩy frame vào hàng đợi với chính sách DROP-OLDEST chống tràn
	void push(FrameData &&frame)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		if (m_count == BUFFER_SIZE) {
			// Hàng đợi đã đầy: Ghi đè lên slot cũ nhất và dịch read_idx
			m_read_idx = (m_read_idx + 1) % BUFFER_SIZE;
			m_dropped_count.fetch_add(1, std::memory_order_relaxed);

		} else {
			m_count++;
		}

		m_buffer[m_write_idx] = std::move(frame);
		m_write_idx = (m_write_idx + 1) % BUFFER_SIZE;

		lock.unlock();
		m_cv.notify_one();
	}

	// Lấy frame mới nhất, có timeout
	bool pop(FrameData &out_frame, std::chrono::milliseconds timeout = std::chrono::milliseconds(50))
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		if (!m_cv.wait_for(lock, timeout, [this] { return m_count > 0; })) {
			return false; // Timeout
		}

		out_frame = std::move(m_buffer[m_read_idx]);
		m_read_idx = (m_read_idx + 1) % BUFFER_SIZE;
		m_count--;

		return true;
	}

	uint64_t getDroppedCount() const
	{
		return m_dropped_count.load(std::memory_order_relaxed);
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_write_idx = 0;
		m_read_idx = 0;
		m_count = 0;
	}

private:
	std::array<FrameData, BUFFER_SIZE> m_buffer;
	size_t m_write_idx;
	size_t m_read_idx;
	size_t m_count;
	std::atomic<uint64_t> m_dropped_count;
	std::mutex m_mutex;
	std::condition_variable m_cv;
};

} // namespace drone
