"""Exercise the actual VisionNode subscription without video or YOLO workers."""
import time
from dataclasses import replace
from types import SimpleNamespace

import pytest

rclpy = pytest.importorskip("rclpy")
from cc_msgs.msg import AiVisionControl, AiVisionTrackPoint
from rclpy.node import Node
from rclpy.parameter import Parameter
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy

from vision_v1.ai_vision_control import AiVisionControlState
from vision_v1.vision_node import VisionNode
from vision_v1.detection_types import Detection, DetectionSnapshot


@pytest.mark.parametrize("preset", ["ultrafast", "superfast", "veryfast"])
def test_encoder_startup_parameters_and_periodic_stage_metrics(monkeypatch, preset):
    monkeypatch.setattr("vision_v1.vision_node.Detector._load_model", lambda self: None)
    monkeypatch.setattr("vision_v1.vision_node.VisionPipeline.start", lambda self: None)
    rclpy.init()
    node = None
    try:
        node = VisionNode(node_name="test_encoder_settings", parameter_overrides=[
            Parameter("encoder_preset", value=preset), Parameter("key_int_max", value=45),
        ])
        assert node.get_parameter("bitrate_kbps").value == 2000
        assert node.publisher.bitrate_kbps == 2000
        assert node.publisher.encoder_preset == preset
        assert node.publisher.key_int_max == 45
        assert all(node.describe_parameter(name).read_only for name in (
            "bitrate_kbps", "encoder_preset", "key_int_max",
        ))
        metrics = replace(
            node.pipeline.metrics(), video_input_frames=20, video_output_frames=19,
            video_rate_limited_frames=1, video_processing_frames=19,
            resize_time_seconds=19 * 0.004, overlay_time_seconds=19 * 0.006,
            push_time_seconds=19 * 0.010,
        )
        monkeypatch.setattr(node.pipeline, "metrics", lambda: metrics)
        monkeypatch.setattr("vision_v1.vision_node.time", SimpleNamespace(
            monotonic=lambda: 101.0, time=time.time,
        ))
        logs = []
        monkeypatch.setattr(node, "get_logger", lambda: SimpleNamespace(info=logs.append))
        node._last_metric_time = 100.0
        node._publish_status()
        assert "video_in_fps=20.0 video_out_fps=19.0" in logs[-1]
        assert "rate_skip=1 rate_skip_fps=1.0 publish_fail=0" in logs[-1]
        assert "resize_ms=4.00 overlay_ms=6.00 push_ms=10.00" in logs[-1]
        node._last_metric_time = 100.0
        node._publish_status()
        assert "rate_skip=0" in logs[-1]
        assert "resize_ms=0.00 overlay_ms=0.00 push_ms=0.00" in logs[-1]
    finally:
        if node is not None:
            node.destroy_node()
        rclpy.shutdown()


def spin_until(node, predicate):
    deadline = time.monotonic() + 3.0
    while not predicate() and time.monotonic() < deadline:
        rclpy.spin_once(node, timeout_sec=0.05)
    assert predicate()


def test_vision_node_receives_live_and_latched_ros_control(monkeypatch):
    monkeypatch.setattr("vision_v1.vision_node.Detector._load_model", lambda self: None)
    monkeypatch.setattr("vision_v1.vision_node.VisionPipeline.start", lambda self: None)
    rclpy.init()
    sender = Node("test_ai_vision_control_sender")
    node = None
    late_node = None
    try:
        qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
        )
        publisher = sender.create_publisher(AiVisionControl, "/cc/ai_vision_control", qos)
        node = VisionNode(node_name="test_vision_control")
        assert node.ai_vision_control.latest() == AiVisionControlState(False, False, False)
        spin_until(sender, lambda: publisher.get_subscription_count() == 1)

        for flags in [(True, False, True), (True, True, False), (False, False, False)]:
            publisher.publish(AiVisionControl(
                timestamp=123, bounding_box=flags[0], tracking=flags[1], following=flags[2]
            ))
            spin_until(node, lambda flags=flags: node.ai_vision_control.latest() == AiVisionControlState(*flags))

        node.destroy_node()
        node = None
        publisher.publish(AiVisionControl(timestamp=124, bounding_box=True, tracking=True, following=True))
        publisher.publish(AiVisionControl(timestamp=125, bounding_box=True, tracking=False, following=True))
        late_node = VisionNode(node_name="test_vision_control_late")
        spin_until(late_node, lambda: late_node.ai_vision_control.latest() == AiVisionControlState(True, False, True))
        subscription = next(
            item for item in late_node.subscriptions if item.topic_name == "/cc/ai_vision_control"
        )
        assert subscription.qos_profile.depth == 1
        assert subscription.qos_profile.history == HistoryPolicy.KEEP_LAST
        assert subscription.qos_profile.reliability == ReliabilityPolicy.RELIABLE
        assert subscription.qos_profile.durability == DurabilityPolicy.TRANSIENT_LOCAL
    finally:
        if node is not None:
            node.destroy_node()
        if late_node is not None:
            late_node.destroy_node()
        sender.destroy_node()
        rclpy.shutdown()


def test_vision_node_selects_from_ros_event_using_control_and_snapshot_age(monkeypatch):
    monkeypatch.setattr("vision_v1.vision_node.Detector._load_model", lambda self: None)
    monkeypatch.setattr("vision_v1.vision_node.VisionPipeline.start", lambda self: None)
    received_points = []
    original_callback = VisionNode._on_ai_vision_track_point

    def on_point(self, message):
        original_callback(self, message)
        received_points.append(message)

    monkeypatch.setattr(VisionNode, "_on_ai_vision_track_point", on_point)
    rclpy.init()
    sender = Node("test_track_point_sender")
    node = None
    try:
        node = VisionNode(node_name="test_track_point_vision")
        assert node.pipeline.ai_vision_control is node.ai_vision_control
        control_pub = sender.create_publisher(AiVisionControl, "/cc/ai_vision_control", QoSProfile(
            depth=1, reliability=ReliabilityPolicy.RELIABLE, durability=DurabilityPolicy.TRANSIENT_LOCAL,
        ))
        point_pub = sender.create_publisher(AiVisionTrackPoint, "/cc/ai_vision_track_point", QoSProfile(
            depth=1, reliability=ReliabilityPolicy.RELIABLE, durability=DurabilityPolicy.VOLATILE,
        ))
        spin_until(sender, lambda: point_pub.get_subscription_count() == 1 and control_pub.get_subscription_count() == 1)

        def click():
            count = len(received_points)
            point_pub.publish(AiVisionTrackPoint(timestamp=123, x=0.405, y=0.43, radius=0.0))
            spin_until(node, lambda: len(received_points) > count)

        def update_snapshot(track_id, age_ns=0):
            node.detection_store.update(DetectionSnapshot.create(
                source_timestamp_us=1, source_width=1000, source_height=600,
                generation=1, source_seq=2,
                detections=[Detection(400, 250, 600, 350, 1, "plant", 0.9, track_id=track_id)],
                completed_monotonic_ns=time.monotonic_ns() - age_ns,
            ))

        update_snapshot(7)
        click()  # Default state ignores the command.
        assert node.ai_vision_control.latest().selected_track_id is None
        control_pub.publish(AiVisionControl(bounding_box=True, tracking=True, following=False))
        spin_until(node, lambda: node.ai_vision_control.latest().tracking)
        click()
        assert node.ai_vision_control.latest().selected_track_id == 7
        click()  # The same ID toggles off once, with no retained event.
        assert node.ai_vision_control.latest().selected_track_id is None
        assert node.ai_vision_control.latest().pending_click is None
        update_snapshot(8)
        click()
        assert node.ai_vision_control.latest().selected_track_id == 8

        from rclpy.parameter import Parameter
        assert node.set_parameters([Parameter("detection_max_age", value=0.01)])[0].successful
        update_snapshot(9, age_ns=20_000_000)
        click()
        assert node.ai_vision_control.latest().selected_track_id == 8
        assert node.ai_vision_control.latest().pending_click is not None

        control_pub.publish(AiVisionControl(bounding_box=True, tracking=False, following=True))
        spin_until(node, lambda: not node.ai_vision_control.latest().tracking)
        click()
        assert node.ai_vision_control.latest().selected_track_id is None
        assert node.ai_vision_control.latest().pending_click is None
        assert node.ai_vision_control.latest().following
        subscription = next(item for item in node.subscriptions if item.topic_name == "/cc/ai_vision_track_point")
        assert subscription.qos_profile.depth == 1
        assert subscription.qos_profile.reliability == ReliabilityPolicy.RELIABLE
        assert subscription.qos_profile.durability == DurabilityPolicy.VOLATILE
    finally:
        if node is not None:
            node.destroy_node()
        sender.destroy_node()
        rclpy.shutdown()
