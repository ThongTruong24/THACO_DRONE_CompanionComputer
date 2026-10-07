#!/usr/bin/env python3
"""Vision ROS 2 Node subscribing to FrameReady and CameraInfo, running YOLO detection."""
import os
import time
import threading
from typing import Optional

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy
from rcl_interfaces.msg import SetParametersResult, ParameterDescriptor

from cc_msgs.msg import FrameReady, VisionStatus
from sensor_msgs.msg import CameraInfo
import sys

try:
    from frame_pool import FramePoolReader
except ImportError:
    _fp_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../lib/frame_pool"))
    if _fp_path not in sys.path:
        sys.path.insert(0, _fp_path)
    from frame_pool import FramePoolReader

from vision.detector import Detector
from vision.publisher import VisionPublisher
from vision.object_geometry import Intrinsics


class Mailbox:
    """Thread-safe single slot mailbox."""
    def __init__(self):
        self.lock = threading.Lock()
        self.value = None
        self.seq = 0

    def put(self, color, depth=None, timestamp=0):
        with self.lock:
            self.seq += 1
            self.value = (self.seq, color, depth, timestamp, time.monotonic())

    def get(self, max_age: float = 2.0):
        with self.lock:
            if self.value is None:
                return None
            _, _, _, _, put_time = self.value
            if time.monotonic() - put_time > max_age:
                return None
            return self.value


class VisionNode(Node):
    def __init__(self, node_name="vision", **kwargs):
        super().__init__(node_name, **kwargs)
        self.declare_parameters()

        # State & metrics
        self.torn_frames_count = 0
        self.latest_detections_count = 0
        self.last_inference_time = 0.0
        self.measured_inference_fps = 0.0
        self.intrinsics: Optional[Intrinsics] = None
        self.has_depth_stream = False

        self.running = True
        self.raw_mailbox = Mailbox()
        self.annotated_mailbox = Mailbox()

        # Initialize reader
        pool_dir = self.get_parameter("pool_dir").value
        self.reader = FramePoolReader(pool_dir)

        # Initialize detector
        model_path = self.get_parameter("model").value
        imgsz = self.get_parameter("imgsz").value
        confidence = self.get_parameter("confidence").value
        self.detector = Detector(model_path, imgsz, confidence)

        # Initialize publisher
        width = self.get_parameter("width").value
        height = self.get_parameter("height").value
        fps = self.get_parameter("fps").value
        bitrate = self.get_parameter("bitrate_kbps").value
        output_url = self.get_parameter("output_url").value
        stale_sec = self.get_parameter("stale_seconds").value
        self.publisher = VisionPublisher(width, height, fps, bitrate, output_url, stale_sec)

        # QoS Profiles
        qos_frame_ready = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.BEST_EFFORT
        )
        qos_camera_info = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL
        )
        qos_status = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE
        )

        # Subscriptions
        self.sub_frame_ready = self.create_subscription(
            FrameReady,
            "/camera/frame_ready",
            self.on_frame_ready,
            qos_frame_ready
        )
        self.sub_camera_info = self.create_subscription(
            CameraInfo,
            "/camera/camera_info",
            self.on_camera_info,
            qos_camera_info
        )

        # Publisher & Timer
        self.pub_status = self.create_publisher(VisionStatus, "/cc/vision_status", qos_status)
        self.status_timer = self.create_timer(1.0, self.publish_status)

        # Parameter validation callback
        self.add_on_set_parameters_callback(self.on_set_parameters)

        # Background worker threads
        self.inference_thread = threading.Thread(target=self.inference_loop, daemon=True, name="inference")
        self.publish_thread = threading.Thread(target=self.publish_loop, daemon=True, name="publish")
        self.inference_thread.start()
        self.publish_thread.start()

        self.get_logger().info("Vision node initialized and running")

    def declare_parameters(self):
        desc_ro = ParameterDescriptor(read_only=True)

        self.declare_parameter("model", "yolo26n.pt")
        self.declare_parameter("confidence", 0.35)
        self.declare_parameter("source", "/camera/frame_ready", desc_ro)
        self.declare_parameter("imgsz", 320, desc_ro)
        self.declare_parameter("inference_fps", 5)
        self.declare_parameter("fps", 15)
        self.declare_parameter("width", 640, desc_ro)
        self.declare_parameter("height", 480, desc_ro)
        self.declare_parameter("bitrate_kbps", 1000)
        self.declare_parameter("output_url", "rtmp://127.0.0.1:1935/yolo", desc_ro)
        self.declare_parameter("stale_seconds", 2.0)
        self.declare_parameter("pool_dir", "/run/frame_pool", desc_ro)

        self.declare_parameter("min_distance_m", 0.3)
        self.declare_parameter("max_distance_m", 10.0)
        self.declare_parameter("sample_radius", 5)

    def on_set_parameters(self, params):
        result = SetParametersResult(successful=True)
        for param in params:
            if param.name == "confidence":
                val = param.value
                if val < 0.05 or val > 0.95:
                    result.successful = False
                    result.reason = "confidence must be between 0.05 and 0.95"
                    return result
                if self.detector:
                    self.detector.set_confidence(val)
            elif param.name == "source":
                result.successful = False
                result.reason = "source is read-only"
                return result
        return result

    def on_camera_info(self, msg: CameraInfo):
        if len(msg.k) >= 9:
            fx, fy = msg.k[0], msg.k[4]
            cx, cy = msg.k[2], msg.k[5]
            if fx > 0 and fy > 0:
                self.intrinsics = Intrinsics(fx=fx, fy=fy, cx=cx, cy=cy)

    def on_frame_ready(self, msg: FrameReady):
        frame = self.reader.read_frame(msg.generation, msg.slot, msg.seq)
        if frame is None:
            self.torn_frames_count += 1
            return

        ts, color, depth = frame
        self.has_depth_stream = (depth is not None)
        self.raw_mailbox.put(color, depth, ts)

    def inference_loop(self):
        last_seq = -1
        inf_fps = self.get_parameter("inference_fps").value
        interval = 1.0 / max(1, inf_fps)
        stale_sec = self.get_parameter("stale_seconds").value

        while self.running:
            start_t = time.monotonic()
            item = self.raw_mailbox.get(max_age=stale_sec)
            if item is None:
                time.sleep(0.02)
                continue

            seq, color, depth, ts, _ = item
            if seq == last_seq:
                time.sleep(0.02)
                continue
            last_seq = seq

            min_dist = self.get_parameter("min_distance_m").value
            max_dist = self.get_parameter("max_distance_m").value
            radius = self.get_parameter("sample_radius").value

            annotated, count = self.detector.detect(
                color_frame=color,
                depth_frame=depth,
                depth_scale=0.001,
                intrinsics=self.intrinsics,
                min_distance_m=min_dist,
                max_distance_m=max_dist,
                sample_radius=radius
            )

            self.latest_detections_count = count
            self.annotated_mailbox.put(annotated, depth, ts)

            elapsed = time.monotonic() - start_t
            if elapsed > 0:
                self.measured_inference_fps = 0.9 * self.measured_inference_fps + 0.1 * (1.0 / elapsed)

            sleep_time = max(0.0, interval - elapsed)
            time.sleep(sleep_time)

    def publish_loop(self):
        last_seq = -1
        fps = self.get_parameter("fps").value
        interval = 1.0 / max(1, fps)
        stale_sec = self.get_parameter("stale_seconds").value

        while self.running:
            start_t = time.monotonic()

            # Check staleness: if no new frame, publisher marks idle and sets pipeline to NULL
            self.publisher.check_stale()

            item = self.annotated_mailbox.get(max_age=stale_sec)
            if item is not None:
                seq, annotated, _, ts, _ = item
                if seq != last_seq:
                    last_seq = seq
                    self.publisher.push_frame(annotated, ts)

            elapsed = time.monotonic() - start_t
            sleep_time = max(0.0, interval - elapsed)
            time.sleep(sleep_time)

    def publish_status(self):
        msg = VisionStatus()
        msg.timestamp = int(time.time() * 1e6)
        msg.confidence_thresh = float(self.get_parameter("confidence").value)
        msg.inference_fps = float(self.measured_inference_fps)
        msg.input_width = int(self.get_parameter("width").value)
        msg.input_height = int(self.get_parameter("height").value)
        msg.video_fps = int(self.get_parameter("fps").value)
        msg.detections_count = int(self.latest_detections_count)

        # status_flags:
        # bit 0: running (1 if active and streaming, 0 if idle)
        # bit 1: depth enabled (1 if depth present)
        flags = 0
        if not self.publisher.is_idle:
            flags |= 0x01
        if self.has_depth_stream:
            flags |= 0x02
        msg.status_flags = flags

        model_name = os.path.basename(self.get_parameter("model").value)
        msg.model_name = model_name[:24]
        msg.input_source = "shm_pool"

        self.pub_status.publish(msg)

    def stop(self):
        self.running = False
        if hasattr(self, "publisher"):
            self.publisher.stop_pipeline()
        if hasattr(self, "reader"):
            self.reader.close()


def main(args=None):
    rclpy.init(args=args)
    node = VisionNode()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, Exception):
        pass
    finally:
        node.stop()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
