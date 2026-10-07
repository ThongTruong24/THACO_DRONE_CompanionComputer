#include "camera_streamer/camera_streamer_node.hpp"
#include "realsense_camera.hpp"
#include "v4l2_camera.hpp"
#include <hrt/hrt.hpp>

#include <chrono>
#include <iostream>

namespace cc
{

CameraStreamerNode::CameraStreamerNode(
	const rclcpp::NodeOptions &options,
	std::unique_ptr<drone::ICameraDevice> camera,
	std::unique_ptr<drone::IGstStreamSink> sink)
	: Node("camera_streamer", options),
	  camera_(std::move(camera)),
	  sink_(std::move(sink))
{
	declare_parameters();

	// Publishers
	pub_status_ = create_publisher<cc_msgs::msg::CameraStatus>("/cc/camera_status", 10);
	pub_frame_ready_ = create_publisher<cc_msgs::msg::FrameReady>("/camera/frame_ready", 10);
	pub_camera_info_ = create_publisher<sensor_msgs::msg::CameraInfo>(
				   "/camera/camera_info",
				   rclcpp::QoS(1).transient_local().reliable()
			   );

	// Status timer 1 Hz
	timer_status_ = create_wall_timer(
				std::chrono::seconds(1),
				std::bind(&CameraStreamerNode::publish_status, this)
			);

	// Parameter validation callback
	param_callback_handle_ = add_on_set_parameters_callback(
					 std::bind(&CameraStreamerNode::on_set_parameters, this, std::placeholders::_1)
				 );

	// Parameter post-set callback (sets flags, stream worker applies under sink lock)
	post_param_callback_handle_ = add_post_set_parameters_callback(
					      std::bind(&CameraStreamerNode::on_post_set_parameters, this, std::placeholders::_1)
				      );

	init_hardware();
}

CameraStreamerNode::~CameraStreamerNode()
{
	stop();
}

void CameraStreamerNode::declare_parameters()
{
	rcl_interfaces::msg::ParameterDescriptor desc_ro;
	desc_ro.read_only = true;

	config_.camera_type = declare_parameter("camera_type", "realsense", desc_ro);
	camera_model_ = declare_parameter("camera_model", "realsense_d435i", desc_ro);

	config_.video.width = declare_parameter("video.width", 1280, desc_ro);
	config_.video.height = declare_parameter("video.height", 720, desc_ro);
	config_.video.rotation = declare_parameter("video.rotation", 180, desc_ro);

	config_.encoder.mode = declare_parameter("encoder.mode", "software", desc_ro);
	config_.encoder.codec = declare_parameter("encoder.codec", "h264", desc_ro);
	config_.encoder.bitrate_min_kbps = declare_parameter("encoder.bitrate_min_kbps", 700, desc_ro);
	config_.encoder.bitrate_max_kbps = declare_parameter("encoder.bitrate_max_kbps", 2000, desc_ro);
	config_.encoder.vbv_buffer_kb = declare_parameter("encoder.vbv_buffer_kb", 1250, desc_ro);
	config_.encoder.tune = declare_parameter("encoder.tune", "zerolatency", desc_ro);
	config_.encoder.speed_preset = declare_parameter("encoder.speed_preset", "ultrafast", desc_ro);
	config_.encoder.threads = declare_parameter("encoder.threads", 4, desc_ro);
	config_.encoder.intra_refresh = declare_parameter("encoder.intra_refresh", false, desc_ro);
	config_.encoder.adaptive_rate = declare_parameter("encoder.adaptive_rate", false, desc_ro);
	config_.encoder.key_int_max = declare_parameter("encoder.key_int_max", 30, desc_ro);
	config_.encoder.leaky_queue_buffers = declare_parameter("encoder.leaky_queue_buffers", 6, desc_ro);

	config_.network.sink_type = declare_parameter("network.sink_type", "rtmp", desc_ro);
	config_.network.target_url = declare_parameter("network.target_url", "rtmp://127.0.0.1:1935/camera", desc_ro);

	config_.realsense.serial_number = declare_parameter("realsense.serial_number", "", desc_ro);
	config_.realsense.profile_mode = declare_parameter("realsense.profile_mode", "rgb_only", desc_ro);
	config_.realsense.enable_emitter = declare_parameter("realsense.enable_emitter", false, desc_ro);
	config_.realsense.depth_width = declare_parameter("realsense.depth_width", 640, desc_ro);
	config_.realsense.depth_height = declare_parameter("realsense.depth_height", 480, desc_ro);
	config_.realsense.depth_fps = declare_parameter("realsense.depth_fps", 30, desc_ro);

	config_.v4l2.device_path = declare_parameter("v4l2.device_path", "/dev/video0", desc_ro);
	config_.v4l2.pixel_format = declare_parameter("v4l2.pixel_format", "YUYV", desc_ro);
	config_.v4l2.num_buffers = declare_parameter("v4l2.num_buffers", 4, desc_ro);

	rtsp_url_qgc_ = declare_parameter("rtsp_url_qgc", "rtsp://127.0.0.1:8554/camera", desc_ro);
	rtsp_url_controller_ = declare_parameter("rtsp_url_controller", "", desc_ro);
	rtsp_url_laptop_ = declare_parameter("rtsp_url_laptop", "", desc_ro);

	// Dynamic parameters
	rcl_interfaces::msg::ParameterDescriptor desc_br;
	desc_br.description = "Live encoding bitrate (300 - 20000 kbps)";
	int br = declare_parameter("bitrate_kbps", 1400, desc_br);
	config_.encoder.bitrate_kbps = br;
	current_bitrate_.store(br);

	rcl_interfaces::msg::ParameterDescriptor desc_fps;
	desc_fps.description = "Camera framerate (1 - 120 fps)";
	int fps = declare_parameter("fps", 30, desc_fps);
	config_.video.fps = fps;
	current_fps_.store(fps);

	bool fpv_en = declare_parameter("fpv_enabled", true);
	fpv_enabled_.store(fpv_en);

	bool ai_en = declare_parameter("ai_enabled", true);
	ai_enabled_.store(ai_en);

	int ai_w = declare_parameter("ai_width", 640);
	int ai_h = declare_parameter("ai_height", 480);
	int ai_f = declare_parameter("ai_fps", 10);
	std::string pool_dir = declare_parameter("ai_pool_dir", "/run/frame_pool", desc_ro);

	AiBranchConfig ai_cfg;
	ai_cfg.enabled = ai_en;
	ai_cfg.width = ai_w;
	ai_cfg.height = ai_h;
	ai_cfg.fps = ai_f;
	ai_cfg.pool_dir = pool_dir;
	ai_branch_ = std::make_unique<AiBranch>(ai_cfg);
}

void CameraStreamerNode::init_hardware()
{
	RCLCPP_INFO(get_logger(), "Camera model: %s (type: %s)", camera_model_.c_str(), config_.camera_type.c_str());
	processor_ = std::make_unique<drone::NeonFrameProcessor>(config_.video.rotation);

	if (!camera_) {
		if (config_.camera_type == "v4l2") {
			camera_ = std::make_unique<drone::V4L2Camera>();

		} else {
			camera_ = std::make_unique<drone::RealSenseCamera>();
		}
	}

	if (!sink_) {
		sink_ = std::make_unique<drone::GstStreamSink>();
	}

	sink_->setEnabled(fpv_enabled_.load());
}

rcl_interfaces::msg::SetParametersResult CameraStreamerNode::on_set_parameters(
	const std::vector<rclcpp::Parameter> &parameters)
{
	rcl_interfaces::msg::SetParametersResult result;
	result.successful = true;

	for (const auto &param : parameters) {
		const auto &name = param.get_name();

		if (name == "bitrate_kbps") {
			if (param.get_type() != rclcpp::ParameterType::PARAMETER_INTEGER) {
				result.successful = false;
				result.reason = "bitrate_kbps must be an integer";
				return result;
			}

			int64_t val = param.as_int();

			if (val < 300 || val > 20000) {
				result.successful = false;
				result.reason = "bitrate_kbps must be between 300 and 20000";
				return result;
			}

		} else if (name == "fps") {
			if (param.get_type() != rclcpp::ParameterType::PARAMETER_INTEGER) {
				result.successful = false;
				result.reason = "fps must be an integer";
				return result;
			}

			int64_t val = param.as_int();

			if (val < 1 || val > 120) {
				result.successful = false;
				result.reason = "fps must be between 1 and 120";
				return result;
			}

		} else if (name == "ai_fps") {
			if (param.get_type() != rclcpp::ParameterType::PARAMETER_INTEGER) {
				result.successful = false;
				result.reason = "ai_fps must be an integer";
				return result;
			}

			int64_t val = param.as_int();

			if (val < 1 || val > 60) {
				result.successful = false;
				result.reason = "ai_fps must be between 1 and 60";
				return result;
			}

		} else if (name == "ai_width") {
			if (param.as_int() <= 0) {
				result.successful = false;
				result.reason = "ai_width must be positive";
				return result;
			}

		} else if (name == "ai_height") {
			if (param.as_int() <= 0) {
				result.successful = false;
				result.reason = "ai_height must be positive";
				return result;
			}

		} else if (name == "fpv_enabled" || name == "ai_enabled") {
			if (param.get_type() != rclcpp::ParameterType::PARAMETER_BOOL) {
				result.successful = false;
				result.reason = name + " must be a boolean";
				return result;
			}
		}
	}

	return result;
}

void CameraStreamerNode::on_post_set_parameters(
	const std::vector<rclcpp::Parameter> &parameters)
{
	for (const auto &param : parameters) {
		const auto &name = param.get_name();

		if (name == "bitrate_kbps") {
			pending_bitrate_.store(static_cast<int>(param.as_int()));

		} else if (name == "fps") {
			pending_fps_.store(static_cast<int>(param.as_int()));

		} else if (name == "fpv_enabled") {
			fpv_enabled_.store(param.as_bool());

		} else if (name == "ai_enabled") {
			ai_enabled_.store(param.as_bool());
			pending_ai_config_flag_.store(true);

		} else if (name == "ai_width" || name == "ai_height" || name == "ai_fps") {
			pending_ai_config_flag_.store(true);
		}
	}
}

void CameraStreamerNode::start()
{
	if (running_.exchange(true)) {
		return;
	}

	capture_thread_ = std::thread(&CameraStreamerNode::capture_loop, this);
	stream_thread_ = std::thread(&CameraStreamerNode::stream_loop, this);
}

void CameraStreamerNode::stop()
{
	if (!running_.exchange(false)) {
		return;
	}

	if (capture_thread_.joinable()) {
		capture_thread_.join();
	}

	if (stream_thread_.joinable()) {
		stream_thread_.join();
	}

	if (sink_) {
		sink_->stop();
	}

	if (camera_ && camera_streaming_.load()) {
		camera_->stopStream();
		camera_streaming_ = false;
	}
}

void CameraStreamerNode::capture_loop()
{
	bool first_info_published = false;

	while (running_.load()) {
		bool want_capture = (fpv_enabled_.load() || ai_enabled_.load());

		if (!want_capture) {
			if (camera_streaming_.exchange(false)) {
				if (camera_) {
					camera_->stopStream();
				}

				RCLCPP_INFO(get_logger(), "Both FPV and AI disabled -> camera stream stopped");
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		if (!camera_streaming_.load()) {
			if (camera_ && camera_->isAvailable()) {
				if (camera_->initialize(config_) && camera_->startStream()) {
					camera_streaming_ = true;
					{
						std::lock_guard<std::mutex> lock(state_mutex_);
						current_device_name_ = camera_->getDeviceName();
					}
					RCLCPP_INFO(get_logger(), "Connected to camera device: %s", current_device_name_.c_str());

					if (fpv_enabled_.load() && sink_) {
						sink_->start(config_);
					}

				} else {
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					continue;
				}

			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				continue;
			}
		}

		drone::FrameData frame;

		if (camera_->captureFrame(frame)) {
			if (!first_info_published) {
				publish_camera_info(frame);
				first_info_published = true;
			}

			ring_buffer_.push(std::move(frame));

		} else {
			auto err = camera_->getLastErrorCode();

			if (err == drone::StreamErrorCode::ERR_USB_IO_ERROR) {
				RCLCPP_WARN(get_logger(), "Camera USB I/O error -> recovering...");
				camera_->stopStream();
				camera_streaming_ = false;
				first_info_published = false;
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
		}
	}
}

void CameraStreamerNode::stream_loop()
{
	while (running_.load()) {
		// 1. Process pending dynamic parameter updates under sink lock
		int pending_br = pending_bitrate_.exchange(-1);

		if (pending_br >= 300) {
			current_bitrate_.store(pending_br);
			config_.encoder.bitrate_kbps = pending_br;

			if (sink_) {
				sink_->setBitrate(pending_br);
			}
		}

		int pending_f = pending_fps_.exchange(-1);

		if (pending_f > 0) {
			current_fps_.store(pending_f);
			config_.video.fps = pending_f;

			if (sink_) {
				sink_->start(config_);
			}
		}

		if (rebuild_pipeline_flag_.exchange(false)) {
			if (sink_) {
				sink_->start(config_);
			}
		}

		if (sink_) {
			sink_->setEnabled(fpv_enabled_.load());
		}

		if (pending_ai_config_flag_.exchange(false)) {
			AiBranchConfig ai_cfg;
			ai_cfg.enabled = ai_enabled_.load();
			ai_cfg.width = static_cast<int>(get_parameter("ai_width").as_int());
			ai_cfg.height = static_cast<int>(get_parameter("ai_height").as_int());
			ai_cfg.fps = static_cast<int>(get_parameter("ai_fps").as_int());
			ai_cfg.pool_dir = get_parameter("ai_pool_dir").as_string();

			if (ai_branch_) {
				ai_branch_->update_config(ai_cfg);
			}

			// Broadcast updated CameraInfo
			drone::FrameData dummy;
			dummy.width = config_.video.width;
			dummy.height = config_.video.height;
			publish_camera_info(dummy);
		}

		// 2. Fetch and process frame
		drone::FrameData frame;

		if (ring_buffer_.pop(frame, std::chrono::milliseconds(40))) {
			// FPV branch
			if (fpv_enabled_.load() && sink_) {
				if (processor_) {
					processor_->process(frame);
				}

				if (!sink_->pushFrame(frame)) {
					if (!sink_->isConnected()) {
						rebuild_pipeline_flag_.store(true);
					}
				}
			}

			// AI branch
			if (ai_enabled_.load() && ai_branch_) {
				bool has_subs = (pub_frame_ready_->get_subscription_count() > 0);
				auto desc_opt = ai_branch_->process_frame(frame, has_subs, hrt_absolute_time());

				if (desc_opt.has_value()) {
					cc_msgs::msg::FrameReady ready_msg;
					ready_msg.timestamp = desc_opt->timestamp;
					ready_msg.generation = desc_opt->generation;
					ready_msg.slot = desc_opt->slot;
					ready_msg.seq = desc_opt->seq;
					pub_frame_ready_->publish(ready_msg);
				}
			}
		}
	}
}

void CameraStreamerNode::publish_camera_info(const drone::FrameData &frame)
{
	if (!ai_branch_ || !pub_camera_info_) {
		return;
	}

	auto info = ai_branch_->get_camera_info(frame);
	info.header.stamp = now();
	info.header.frame_id = "camera_optical_frame";
	pub_camera_info_->publish(info);
}

void CameraStreamerNode::publish_status()
{
	cc_msgs::msg::CameraStatus msg;
	msg.timestamp = hrt_absolute_time();

	msg.video_width = static_cast<uint16_t>(config_.video.width);
	msg.video_height = static_cast<uint16_t>(config_.video.height);
	msg.depth_width = static_cast<uint16_t>(config_.realsense.depth_width);
	msg.depth_height = static_cast<uint16_t>(config_.realsense.depth_height);
	msg.rotation = static_cast<uint16_t>(config_.video.rotation);

	msg.bitrate_kbps = static_cast<uint16_t>(sink_ ? sink_->getCurrentBitrate() : current_bitrate_.load());
	msg.bitrate_max_kbps = static_cast<uint16_t>(config_.encoder.bitrate_max_kbps);
	msg.vbv_buffer_kb = static_cast<uint16_t>(config_.encoder.vbv_buffer_kb);
	msg.rtsp_port = 8554;

	msg.video_fps = static_cast<uint8_t>(current_fps_.load());
	msg.depth_fps = static_cast<uint8_t>(config_.realsense.depth_fps);
	msg.camera_id = 1;
	msg.is_default = 1;
	msg.profile_mode = (config_.realsense.profile_mode == "rgb_depth") ? 1 : 0;
	msg.enable_emitter = config_.realsense.enable_emitter ? 1 : 0;

	// camera_status: 0 = offline/disconnected, 1 = standby, 2 = streaming
	if (!fpv_enabled_.load() && !ai_enabled_.load()) {
		msg.camera_status = 1;

	} else if (!camera_streaming_.load()) {
		msg.camera_status = 0;

	} else {
		msg.camera_status = 2;
	}

	msg.error_code = camera_ ? static_cast<uint8_t>(camera_->getLastErrorCode()) : 0;
	msg.usb_speed_mode = 2;

	{
		std::lock_guard<std::mutex> lock(state_mutex_);
		msg.camera_name = current_device_name_;
	}
	msg.camera_type = config_.camera_type;
	msg.connection_port = "USB3";
	msg.serial_number = config_.realsense.serial_number;
	msg.codec = config_.encoder.codec;
	msg.encoder_mode = config_.encoder.mode;

	msg.rtsp_url_qgc = rtsp_url_qgc_;
	msg.rtsp_url_controller = rtsp_url_controller_;
	msg.rtsp_url_laptop = rtsp_url_laptop_;

	msg.fpv_enabled = fpv_enabled_.load();
	msg.ai_enabled = ai_enabled_.load();

	pub_status_->publish(msg);
}

std::string CameraStreamerNode::get_status_string() const
{
	if (!fpv_enabled_.load() && !ai_enabled_.load()) {
		return "STANDBY";
	}

	if (!camera_streaming_.load()) {
		return "DISCONNECTED";
	}

	return "STREAMING";
}

} // namespace cc
