import threading
import time

from vision_v1.detection_store import DetectionStore
from vision_v1.detection_types import DetectionSnapshot
from vision_v1.frame_pool_source import InferenceFrame
from vision_v1.latest_value import LatestValue
from vision_v1.pipeline import VisionPipeline


class FakeFrame:
    shape = (2, 2, 3)


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
        output_width=2,
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
