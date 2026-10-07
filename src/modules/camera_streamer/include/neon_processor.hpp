#pragma once

#include "camera_interface.hpp"
#include <atomic>

namespace drone
{

class NeonFrameProcessor : public IFrameProcessor
{
public:
	explicit NeonFrameProcessor(int rotation = 180) : m_rotation(rotation) {}

	void process(FrameData &frame) override;
	void setRotation(int rotation) override { m_rotation.store(rotation); }
	int getRotation() const override { return m_rotation.load(); }

private:
	void rotate180_inplace(uint8_t *data, int width, int height);
	std::atomic<int> m_rotation;
};

} // namespace drone
