#pragma once

#include <rclcpp/rclcpp.hpp>
#include <cc_msgs/msg/camera_status.hpp>
#include <cc_msgs/msg/frame_ready.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include "ICameraDevice.hpp"
#include "gst_stream_sink.hpp"
#include "safe_ring_buffer.hpp"
#include "neon_processor.hpp"
#include "camera_streamer/ai_branch.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

namespace cc
{

class CameraStreamerNode : public rclcpp::Node
{
public:
	explicit CameraStreamerNode(
		const rclcpp::NodeOptions &options = rclcpp::NodeOptions(),
		std::unique_ptr<drone::ICameraDevice> camera = nullptr,
		std::unique_ptr<drone::IGstStreamSink> sink = nullptr);
	~CameraStreamerNode() override;

	void start();
	void stop();

	// Accessors for unit testing & monitoring
	bool is_fpv_enabled() const { return fpv_enabled_.load(); }
	bool is_ai_enabled() const { return ai_enabled_.load(); }
	bool is_camera_streaming() const { return camera_streaming_.load(); }
	int get_current_bitrate() const { return current_bitrate_.load(); }
	int get_current_fps() const { return current_fps_.load(); }
	std::string get_status_string() const;

	drone::ICameraDevice *camera_device() const { return camera_.get(); }
	drone::IGstStreamSink *stream_sink() const { return sink_.get(); }
	AiBranch *ai_branch() const { return ai_branch_.get(); }
	const std::string &get_camera_model() const { return camera_model_; }

private:
	void declare_parameters();
	rcl_interfaces::msg::SetParametersResult on_set_parameters(
		const std::vector<rclcpp::Parameter> &parameters);
	void on_post_set_parameters(
		const std::vector<rclcpp::Parameter> &parameters);

	void init_hardware();
	void publish_status();
	void publish_camera_info(const drone::FrameData &frame);

	void capture_loop();
	void stream_loop();

	std::unique_ptr<drone::ICameraDevice> camera_;
	std::unique_ptr<drone::IGstStreamSink> sink_;
	std::unique_ptr<drone::NeonFrameProcessor> processor_;
	std::unique_ptr<AiBranch> ai_branch_;
	drone::SafeRingBuffer ring_buffer_;

	drone::CameraConfig config_;

	std::atomic<bool> running_{false};
	std::atomic<bool> fpv_enabled_{true};
	std::atomic<bool> ai_enabled_{true};
	std::atomic<bool> camera_streaming_{false};
	std::atomic<int> current_bitrate_{1400};
	std::atomic<int> current_fps_{30};
	std::atomic<bool> rebuild_pipeline_flag_{false};

	// Pending runtime parameter changes applied in stream worker
	std::atomic<int> pending_bitrate_{-1};
	std::atomic<int> pending_fps_{-1};
	std::atomic<bool> pending_ai_config_flag_{false};

	std::thread capture_thread_;
	std::thread stream_thread_;

	rclcpp::Publisher<cc_msgs::msg::CameraStatus>::SharedPtr pub_status_;
	rclcpp::Publisher<cc_msgs::msg::FrameReady>::SharedPtr pub_frame_ready_;
	rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr pub_camera_info_;
	rclcpp::TimerBase::SharedPtr timer_status_;

	rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
	rclcpp::node_interfaces::PostSetParametersCallbackHandle::SharedPtr post_param_callback_handle_;

	mutable std::mutex state_mutex_;
	std::string rtsp_url_qgc_;
	std::string rtsp_url_controller_;
	std::string rtsp_url_laptop_;
	std::string current_device_name_{"None"};
	std::string camera_model_{"realsense_d435i"};
};

} // namespace cc
