#!/usr/bin/env python3
"""ROS 2 adapter for the decoupled vision_v1 pipeline."""
from __future__ import annotations

import os
import threading
import time

import rclpy
from rcl_interfaces.msg import ParameterDescriptor, SetParametersResult
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import CameraInfo

from cc_msgs.msg import FrameReady, VisionStatus

from .detection_store import DetectionStore
from .detector import Detector
from .frame_pool_source import FramePoolInferenceSource
from .object_geometry import Intrinsics
from .overlay import OverlayRenderer
from .pipeline import VisionPipeline
from .publisher import VisionPublisher
from .rtsp_source import RtspVideoSource
from .stream_validation import validate_stream_routes


class VisionNode(Node):
    def __init__(self, node_name: str = "vision", **kwargs):
        super().__init__(node_name, **kwargs)
        self._stop_lock = threading.Lock()
        self._stopped = False
        self._declare_parameters()
        self._validate_startup_config()

        self.detector = Detector(
            self.get_parameter("model").value,
            self.get_parameter("imgsz").value,
            self.get_parameter("confidence").value,
        )
        self.inference_source = FramePoolInferenceSource(self.get_parameter("pool_dir").value)
        self.video_source = RtspVideoSource(
            self.get_parameter("input_url").value,
            reconnect_seconds=self.get_parameter("reconnect_seconds").value,
        )
        self.detection_store = DetectionStore()
        self.overlay = OverlayRenderer(self.get_parameter("detection_max_age").value)
        self.publisher = VisionPublisher(
            self.get_parameter("width").value,
            self.get_parameter("height").value,
            self.get_parameter("fps").value,
            self.get_parameter("bitrate_kbps").value,
            self.get_parameter("output_url").value,
            self.get_parameter("stale_seconds").value,
        )
        self.pipeline = VisionPipeline(
            video_source=self.video_source,
            inference_source=self.inference_source,
            detector=self.detector,
            detection_store=self.detection_store,
            overlay=self.overlay,
            publisher=self.publisher,
            output_width=self.get_parameter("width").value,
            output_height=self.get_parameter("height").value,
            video_fps_getter=lambda: self.get_parameter("fps").value,
            inference_fps_getter=lambda: self.get_parameter("inference_fps").value,
            detection_max_age_getter=lambda: self.get_parameter("detection_max_age").value,
            enabled_getter=lambda: self.get_parameter("enabled").value,
            depth_options_getter=self._depth_options,
        )

        frame_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.BEST_EFFORT,
        )
        camera_info_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
        )
        status_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
        )
        self.create_subscription(FrameReady, "/camera/frame_ready", self._on_frame_ready, frame_qos)
        self.create_subscription(CameraInfo, "/camera/camera_info", self._on_camera_info, camera_info_qos)
        self.status_publisher = self.create_publisher(VisionStatus, "/cc/vision_status", status_qos)
        self.status_timer = self.create_timer(1.0, self._publish_status)
        self.add_on_set_parameters_callback(self._on_set_parameters)

        self._last_metric_time = time.monotonic()
        self._last_video_input = 0
        self._last_video_output = 0
        self._last_inference_completed = 0
        self._video_input_fps = 0.0
        self._video_output_fps = 0.0
        self._inference_completion_fps = 0.0
        self.pipeline.start()
        self.get_logger().info("vision_v1 started: RTSP video path is independent from YOLO inference")

    def _declare_parameters(self) -> None:
        read_only = ParameterDescriptor(read_only=True)
        self.declare_parameter("enabled", True)
        self.declare_parameter("camera_model", "realsense_d435i", read_only)
        self.declare_parameter("model", "yolo26n.pt", read_only)
        self.declare_parameter("confidence", 0.35)
        self.declare_parameter("source", "/camera/frame_ready", read_only)
        self.declare_parameter("input_url", "rtsp://127.0.0.1:8554/camera", read_only)
        self.declare_parameter("imgsz", 320, read_only)
        self.declare_parameter("inference_fps", 5)
        self.declare_parameter("fps", 30)
        self.declare_parameter("width", 640, read_only)
        self.declare_parameter("height", 480, read_only)
        self.declare_parameter("bitrate_kbps", 1000, read_only)
        self.declare_parameter("output_url", "rtmp://127.0.0.1:1935/yolo", read_only)
        self.declare_parameter("stale_seconds", 2.0)
        self.declare_parameter("detection_max_age", 0.5)
        self.declare_parameter("reconnect_seconds", 1.0, read_only)
        self.declare_parameter("pool_dir", "/run/frame_pool", read_only)
        self.declare_parameter("min_distance_m", 0.3)
        self.declare_parameter("max_distance_m", 10.0)
        self.declare_parameter("sample_radius", 5)

    def _validate_startup_config(self) -> None:
        input_url = self.get_parameter("input_url").value
        output_url = self.get_parameter("output_url").value
        error = validate_stream_routes(input_url, output_url)
        if error:
            raise ValueError(error)

    def _depth_options(self) -> dict:
        return {
            "min_distance_m": self.get_parameter("min_distance_m").value,
            "max_distance_m": self.get_parameter("max_distance_m").value,
            "sample_radius": self.get_parameter("sample_radius").value,
        }

    def _on_frame_ready(self, message: FrameReady) -> None:
        self.inference_source.submit_descriptor(message.generation, message.slot, message.seq)

    def _on_camera_info(self, message: CameraInfo) -> None:
        if len(message.k) >= 9 and message.k[0] > 0 and message.k[4] > 0:
            self.pipeline.set_intrinsics(
                Intrinsics(fx=message.k[0], fy=message.k[4], cx=message.k[2], cy=message.k[5])
            )

    def _on_set_parameters(self, parameters):
        for parameter in parameters:
            value = parameter.value
            if parameter.name == "confidence" and not 0.05 <= value <= 0.95:
                return SetParametersResult(successful=False, reason="confidence must be in [0.05, 0.95]")
            elif parameter.name == "inference_fps" and not 1 <= value <= 30:
                return SetParametersResult(successful=False, reason="inference_fps must be in [1, 30]")
            elif parameter.name == "fps" and not 1 <= value <= 60:
                return SetParametersResult(successful=False, reason="fps must be in [1, 60]")
            elif parameter.name in ("stale_seconds", "detection_max_age") and value <= 0:
                return SetParametersResult(successful=False, reason=f"{parameter.name} must be positive")
            elif parameter.name == "sample_radius" and value < 0:
                return SetParametersResult(successful=False, reason="sample_radius must be non-negative")

        for parameter in parameters:
            if parameter.name == "confidence":
                self.detector.set_confidence(parameter.value)
            elif parameter.name == "fps":
                self.publisher.set_fps(parameter.value)
            elif parameter.name == "stale_seconds":
                self.publisher.stale_seconds = float(parameter.value)
        return SetParametersResult(successful=True)

    def _publish_status(self) -> None:
        self.pipeline.check_output_stale()
        metrics = self.pipeline.metrics()
        now = time.monotonic()
        elapsed = max(1e-6, now - self._last_metric_time)
        self._video_input_fps = (metrics.video_input_frames - self._last_video_input) / elapsed
        self._video_output_fps = (metrics.video_output_frames - self._last_video_output) / elapsed
        self._inference_completion_fps = (
            metrics.inference_completed - self._last_inference_completed
        ) / elapsed
        self._last_metric_time = now
        self._last_video_input = metrics.video_input_frames
        self._last_video_output = metrics.video_output_frames
        self._last_inference_completed = metrics.inference_completed

        snapshot = self.detection_store.latest()
        message = VisionStatus()
        message.timestamp = int(time.time() * 1_000_000)
        message.confidence_thresh = float(self.get_parameter("confidence").value)
        message.inference_fps = float(self._inference_completion_fps)
        message.input_width = int(self.get_parameter("width").value)
        message.input_height = int(self.get_parameter("height").value)
        message.video_fps = min(255, max(0, round(self._video_output_fps)))
        message.detections_count = 0 if snapshot is None else min(255, len(snapshot.detections))
        flags = 0
        if not self.publisher.is_idle:
            flags |= 0x01
        if snapshot is not None and any(item.distance_m is not None for item in snapshot.detections):
            flags |= 0x02
        message.status_flags = flags
        message.model_name = os.path.basename(self.get_parameter("model").value)[:24]
        message.input_source = "rtsp+shm"
        self.status_publisher.publish(message)

        age = "none" if metrics.snapshot_age_seconds is None else f"{metrics.snapshot_age_seconds:.3f}s"
        self.get_logger().info(
            f"metrics video_in_fps={self._video_input_fps:.1f} "
            f"video_out_fps={self._video_output_fps:.1f} "
            f"inference_fps={self._inference_completion_fps:.1f} "
            f"drop_video={metrics.dropped_video_frames} "
            f"drop_inference={metrics.dropped_inference_frames} "
            f"snapshot_age={age}"
        )

    def stop(self) -> None:
        with self._stop_lock:
            if self._stopped:
                return
            self._stopped = True
        self.pipeline.stop()

    def destroy_node(self):
        self.stop()
        return super().destroy_node()


def main(args=None) -> None:
    rclpy.init(args=args)
    node = VisionNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.stop()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
