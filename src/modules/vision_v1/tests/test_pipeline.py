import threading
import time
from types import SimpleNamespace

import pytest

from vision_v1.detection_store import DetectionStore
from vision_v1.detection_types import DetectionSnapshot
from vision_v1.frame_pool_source import InferenceFrame
from vision_v1.latest_value import LatestValue
from vision_v1.pipeline import VisionPipeline


class FakeFrame:
    shape = (2, 4, 3)


class ManualSource:
    def __init__(self, closable_reader=False):
        self.mailbox = LatestValue()
        self.received_frames = 0
        self._alive = False
        self.reader_closed = not closable_reader

    def start(self):
        self._alive = False

    def put(self, value):
        self.received_frames += 1
        self.mailbox.put(value)

    def wait_next(self, version, timeout=None):
        return self.mailbox.wait_next(version, timeout)

    def signal_stop(self):
        self.mailbox.close()

    def join(self, timeout=None):
        pass

    def close_reader(self):
        self.reader_closed = True

    @property
    def dropped_frames(self):
        return self.mailbox.dropped

    @property
    def is_alive(self):
        return self._alive


class SlowDetector:
    def __init__(self):
        self.started = threading.Event()
        self.release = threading.Event()

    def infer(self, frame, **kwargs):
        self.thread_name = threading.current_thread().name
        self.started.set()
        self.release.wait(2.0)
        return DetectionSnapshot.create(
            source_timestamp_us=kwargs["source_timestamp_us"],
            source_width=2,
            source_height=2,
            generation=kwargs["generation"],
            source_seq=kwargs["source_seq"],
            detections=(),
        )


class PassthroughOverlay:
    detection_max_age = 1.0

    def __init__(self):
        self.controls = []

    def render(self, frame, snapshot, copy_frame=False, control=None):
        self.controls.append(control)
        return frame, snapshot is not None


class RecordingPublisher:
    def __init__(self):
        self.frames = []
        self.stopped = False

    def push_frame(self, frame):
        self.frames.append(frame)
        return True

    def stop_pipeline(self):
        self.stopped = True

    def check_stale(self):
        return False


def wait_until(predicate, timeout=1.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.005)
    return predicate()


def build_pipeline(detector):
    video = ManualSource()
    inference = ManualSource(closable_reader=True)
    publisher = RecordingPublisher()
    pipeline = VisionPipeline(
        video_source=video,
        inference_source=inference,
        detector=detector,
        detection_store=DetectionStore(),
        overlay=PassthroughOverlay(),
        publisher=publisher,
        output_width=4,
        output_height=2,
        video_fps_getter=lambda: 1000,
        inference_fps_getter=lambda: 5,
        detection_max_age_getter=lambda: 0.5,
        depth_options_getter=lambda: {
            "min_distance_m": 0.3,
            "max_distance_m": 10.0,
            "sample_radius": 5,
        },
        resize_frame=lambda frame, width, height: frame,
    )
    return pipeline, video, inference, publisher


def test_slow_inference_does_not_block_video_publishing():
    detector = SlowDetector()
    pipeline, video, inference, publisher = build_pipeline(detector)
    pipeline.start()
    inference.put(InferenceFrame(1, 1, 2, FakeFrame(), None, 0.001))
    assert detector.started.wait(0.5)

    for _ in range(6):
        video.put(FakeFrame())
        time.sleep(0.005)

    assert wait_until(lambda: len(publisher.frames) >= 2)
    assert not detector.release.is_set()
    detector.release.set()
    pipeline.stop()


def test_clean_shutdown_joins_workers_and_closes_resources():
    detector = SlowDetector()
    detector.release.set()
    pipeline, _, inference, publisher = build_pipeline(detector)
    pipeline.start()
    pipeline.stop()
    assert not pipeline.is_alive
    assert inference.reader_closed
    assert publisher.stopped


def test_control_changes_do_not_stop_video_or_inference():
    detector = SlowDetector()
    pipeline, video, inference, publisher = build_pipeline(detector)
    pipeline.start()
    try:
        # Even with the default bbox OFF, inference starts and video is published.
        inference.put(InferenceFrame(1, 1, 2, FakeFrame(), None, 0.001))
        assert detector.started.wait(0.5)
        assert detector.thread_name == "vision-inference"
        video.put(FakeFrame())
        assert wait_until(lambda: len(publisher.frames) >= 1)
        assert not pipeline.overlay.controls[-1].bounding_box
        pipeline.ai_vision_control.update(True, True, True)
        video.put(FakeFrame())
        assert wait_until(lambda: len(publisher.frames) >= 2)
        assert pipeline.overlay.controls[-1].tracking
        pipeline.ai_vision_control.update(False, True, True)
        time.sleep(0.005)
        video.put(FakeFrame())
        assert wait_until(lambda: len(publisher.frames) >= 3)
        assert not pipeline.overlay.controls[-1].bounding_box
        assert not pipeline.overlay.controls[-1].tracking
        detector.release.set()
        assert wait_until(lambda: pipeline.metrics().inference_completed == 1)
    finally:
        detector.release.set()
        pipeline.stop()


def test_video_geometry_changes_without_waiting_for_inference():
    import cv2  # Load the native module before starting workers (also under ARM emulation).
    import numpy as np

    assert cv2.resize is not None
    detector = SlowDetector()
    pipeline, video, inference, publisher = build_pipeline(detector)
    pipeline.output_width, pipeline.output_height = 640, 480
    pipeline.video_fps_getter = lambda: 1e9  # Geometry test must not drop a frame due to the rate limiter.
    from vision_v1.pipeline import _resize_frame
    pipeline.resize_frame = _resize_frame
    pipeline.start()
    try:
        inference.put(InferenceFrame(1, 1, 2, FakeFrame(), None, 0.001))
        assert detector.started.wait(0.5)
        for width, height, expected in [(1280, 720, (360, 640, 3)),
                                        (640, 480, (480, 640, 3)),
                                        (320, 240, (240, 320, 3))]:
            previous = len(publisher.frames)
            video.put(np.zeros((height, width, 3), dtype=np.uint8))
            assert wait_until(lambda previous=previous: len(publisher.frames) > previous, timeout=10)
            assert publisher.frames[-1].shape == expected
            assert pipeline.metrics().input_width == width
            assert pipeline.metrics().output_height == expected[0]
        assert not detector.release.is_set()
    finally:
        detector.release.set()
        pipeline.stop()


def test_inference_worker_resolves_pending_click():
    from vision_v1.detection_types import Detection

    class ReturningDetector:
        def infer(self, frame, **kwargs):
            return DetectionSnapshot.create(
                source_timestamp_us=1, source_width=640, source_height=480,
                generation=1, source_seq=2,
                detections=[Detection(100, 100, 300, 300, 0, "plant", 0.9, track_id=7)],
            )

    pipeline, video, inference, publisher = build_pipeline(ReturningDetector())
    pipeline.ai_vision_control.update(True, True, False)
    pipeline.ai_vision_control.select_track_point(110 / 640, 110 / 480, 0, None, max_age_seconds=0.5)
    pipeline.start()
    try:
        inference.put(InferenceFrame(1, 1, 2, FakeFrame(), None, 0.001))
        assert wait_until(lambda: pipeline.ai_vision_control.latest().selected_track_id == 7)
        assert pipeline.ai_vision_control.latest().pending_click is None
        video.put(FakeFrame())
        assert wait_until(lambda: bool(publisher.frames))
        assert pipeline.overlay.controls[-1].selected_track_id == 7
    finally:
        pipeline.stop()


def run_clocked_video(monkeypatch, arrivals, fps, stage_seconds=(0.004, 0.006, 0.010),
                      first_push_delay=0.0, push_ok=True):
    """Run the actual video loop with virtual arrivals and latest-frame drops."""
    pipeline, _, _, publisher = build_pipeline(SlowDetector())
    clock = SimpleNamespace(now=100.0)
    monkeypatch.setattr("vision_v1.pipeline.time", SimpleNamespace(monotonic=lambda: clock.now))
    pipeline.video_fps_getter = (lambda: fps(clock.now - 100.0)) if callable(fps) else lambda: fps
    attempts = []

    class ClockedSource:
        received_frames = len(arrivals)
        dropped_frames = 0

        def __init__(self):
            self.index = 0

        def wait_next(self, last_version, timeout=None):
            if self.index == len(arrivals):
                pipeline._stop_event.set()
                return None
            # During processing only the newest arrived frame survives.
            while self.index + 1 < len(arrivals) and 100.0 + arrivals[self.index + 1] <= clock.now:
                self.index += 1
                self.dropped_frames += 1
            clock.now = max(clock.now, 100.0 + arrivals[self.index])
            self.index += 1
            return self.index, SimpleNamespace(shape=(4, 8, 3))

    def resize(frame, width, height):
        attempts.append(clock.now - 100.0)
        clock.now += stage_seconds[0]
        return FakeFrame()

    original_render = pipeline.overlay.render

    def render(*args, **kwargs):
        clock.now += stage_seconds[1]
        return original_render(*args, **kwargs)

    def push(frame):
        clock.now += stage_seconds[2] + (first_push_delay if len(attempts) == 1 else 0)
        if push_ok:
            publisher.frames.append(frame)
        return push_ok

    pipeline.video_source = ClockedSource()
    pipeline.resize_frame = resize
    pipeline.overlay.render = render
    publisher.push_frame = push
    pipeline._video_loop()
    return pipeline.metrics(), attempts


def test_20_fps_input_with_20_ms_processing_is_not_halved(monkeypatch):
    metrics, attempts = run_clocked_video(monkeypatch, [i / 20 for i in range(40)], fps=30)
    assert len(attempts) == metrics.video_output_frames == metrics.video_input_frames == 40
    assert metrics.video_rate_limited_frames == metrics.dropped_video_frames == 0
    assert metrics.video_publish_failures == 0


def test_60_fps_input_is_limited_to_30_without_bursts(monkeypatch):
    metrics, attempts = run_clocked_video(monkeypatch, [i / 60 for i in range(120)], fps=30)
    assert len(attempts) == metrics.video_output_frames == 60
    assert metrics.video_rate_limited_frames == 60
    assert all(b - a >= 1 / 30 - 1e-9 for a, b in zip(attempts, attempts[1:]))


def test_late_processing_rebases_deadline_and_uses_latest_frame(monkeypatch):
    metrics, attempts = run_clocked_video(
        monkeypatch, [i / 60 for i in range(60)], fps=30,
        stage_seconds=(0, 0, 0), first_push_delay=0.120,
    )
    assert attempts[:2] == pytest.approx([0, 0.120])
    assert metrics.dropped_video_frames > 0
    assert all(b - a >= 1 / 30 - 1e-9 for a, b in zip(attempts, attempts[1:]))
    assert metrics.video_input_frames == (
        metrics.video_output_frames + metrics.video_rate_limited_frames + metrics.dropped_video_frames
    )


def test_video_metrics_measure_stages_including_failed_pushes(monkeypatch):
    metrics, attempts = run_clocked_video(
        monkeypatch, [i / 20 for i in range(20)], fps=30, push_ok=False,
    )
    assert metrics.video_output_frames == 0
    assert metrics.video_publish_failures == metrics.video_processing_frames == len(attempts) == 20
    assert metrics.resize_time_seconds == pytest.approx(20 * 0.004)
    assert metrics.overlay_time_seconds == pytest.approx(20 * 0.006)
    assert metrics.push_time_seconds == pytest.approx(20 * 0.010)


def test_fps_change_resets_deadline_without_completion_based_pacing(monkeypatch):
    metrics, attempts = run_clocked_video(
        monkeypatch, [i / 60 for i in range(120)],
        fps=lambda elapsed: 60 if elapsed < 0.5 else 15,
        stage_seconds=(0, 0, 0),
    )
    assert attempts[:30] == pytest.approx([i / 60 for i in range(30)])
    assert all(b - a >= 1 / 15 - 1e-9 for a, b in zip(attempts[30:], attempts[31:]))
    assert metrics.video_output_frames == len(attempts)
