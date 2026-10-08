"""ROS-independent orchestration for decoupled video and inference workers."""
from __future__ import annotations

from dataclasses import dataclass
import logging
import threading
import time
from typing import Callable, Optional

from .detection_store import DetectionStore
from .ai_vision_control import AiVisionControlStore
from .video_geometry import fit_output_size


LOG = logging.getLogger("vision_v1.pipeline")


@dataclass(frozen=True)
class PipelineMetrics:
    video_input_frames: int
    video_output_frames: int
    inference_completed: int
    video_publish_failures: int
    dropped_video_frames: int
    dropped_inference_frames: int
    snapshot_age_seconds: Optional[float]
    input_width: int
    input_height: int
    output_width: int
    output_height: int
    video_rate_limited_frames: int
    video_processing_frames: int
    resize_time_seconds: float
    overlay_time_seconds: float
    push_time_seconds: float


class VisionPipeline:
    def __init__(
        self,
        *,
        video_source,
        inference_source,
        detector,
        detection_store: DetectionStore,
        overlay,
        publisher,
        output_width: int,
        output_height: int,
        video_fps_getter: Callable[[], float],
        inference_fps_getter: Callable[[], float],
        detection_max_age_getter: Callable[[], float],
        depth_options_getter: Callable[[], dict],
        enabled_getter: Callable[[], bool] = lambda: True,
        resize_frame: Optional[Callable[[object, int, int], object]] = None,
        ai_vision_control: Optional[AiVisionControlStore] = None,
    ):
        self.video_source = video_source
        self.inference_source = inference_source
        self.detector = detector
        self.detection_store = detection_store
        self.overlay = overlay
        self.publisher = publisher
        self.output_width = int(output_width)
        self.output_height = int(output_height)
        if min(self.output_width, self.output_height) <= 0:
            raise ValueError("output bounds must be positive")
        self.video_fps_getter = video_fps_getter
        self.inference_fps_getter = inference_fps_getter
        self.detection_max_age_getter = detection_max_age_getter
        self.depth_options_getter = depth_options_getter
        self.enabled_getter = enabled_getter
        self.resize_frame = resize_frame or _resize_frame
        self.ai_vision_control = ai_vision_control or AiVisionControlStore()

        self._stop_event = threading.Event()
        self._inference_thread: Optional[threading.Thread] = None
        self._video_thread: Optional[threading.Thread] = None
        self._metrics_lock = threading.Lock()
        self._video_output_frames = 0
        self._inference_completed = 0
        self._video_publish_failures = 0
        self._video_rate_limited_frames = 0
        self._video_processing_frames = 0
        self._resize_time_seconds = 0.0
        self._overlay_time_seconds = 0.0
        self._push_time_seconds = 0.0
        self._input_size = (0, 0)
        self._output_size = (0, 0)
        self._last_bad_size = None
        self._intrinsics = None
        self._intrinsics_lock = threading.Lock()

    def start(self) -> None:
        if self.is_alive:
            return
        self._stop_event.clear()
        self._inference_thread = threading.Thread(
            target=self._inference_loop,
            name="vision-inference",
            daemon=False,
        )
        self._video_thread = threading.Thread(
            target=self._video_loop,
            name="vision-video",
            daemon=False,
        )
        self._inference_thread.start()
        self._video_thread.start()
        self.video_source.start()

    def set_intrinsics(self, intrinsics) -> None:
        with self._intrinsics_lock:
            self._intrinsics = intrinsics

    def _inference_loop(self) -> None:
        last_version = 0
        next_allowed = 0.0
        while not self._stop_event.is_set():
            now = time.monotonic()
            if now < next_allowed and self._stop_event.wait(next_allowed - now):
                break
            item = self.inference_source.wait_next(last_version, timeout=0.2)
            if item is None:
                continue
            if self._stop_event.is_set():
                break
            last_version, frame = item
            if not self.enabled_getter():
                continue

            started_at = time.monotonic()

            with self._intrinsics_lock:
                intrinsics = self._intrinsics
            options = self.depth_options_getter()
            snapshot = self.detector.infer(
                frame.color,
                source_timestamp_us=frame.source_timestamp_us,
                generation=frame.generation,
                source_seq=frame.source_seq,
                depth_frame=frame.depth,
                depth_scale=frame.depth_scale,
                intrinsics=intrinsics,
                **options,
            )
            if self._stop_event.is_set():
                break
            self.detection_store.update(snapshot)
            self.ai_vision_control.on_detection_snapshot(
                snapshot, max_age_seconds=float(self.detection_max_age_getter()),
            )
            with self._metrics_lock:
                self._inference_completed += 1
            next_allowed = started_at + 1.0 / max(0.1, float(self.inference_fps_getter()))

    def _video_loop(self) -> None:
        last_version = 0
        next_publish_deadline = 0.0
        previous_interval = None
        while not self._stop_event.is_set():
            item = self.video_source.wait_next(last_version, timeout=0.2)
            if item is None:
                continue
            if self._stop_event.is_set():
                break
            last_version, frame = item
            if not self.enabled_getter():
                continue

            now = time.monotonic()
            interval = 1.0 / max(1.0, float(self.video_fps_getter()))
            if interval != previous_interval:
                next_publish_deadline = now
                previous_interval = interval
            if now + 1e-9 < next_publish_deadline:
                with self._metrics_lock:
                    self._video_rate_limited_frames += 1
                continue
            # Anchor to processing START, never push completion. A late frame
            # rebases the deadline instead of consuming missed slots in a burst.
            # No sleep or backlog: the next iteration reads the latest mailbox.
            next_publish_deadline = now + interval

            height, width = frame.shape[:2]
            try:
                output_size = fit_output_size(width, height, self.output_width, self.output_height)
            except ValueError as exc:
                if self._last_bad_size != (width, height):
                    LOG.error("Cannot publish source %sx%s: %s", width, height, exc)
                    self._last_bad_size = (width, height)
                with self._metrics_lock:
                    self._video_publish_failures += 1
                continue
            self._last_bad_size = None
            with self._metrics_lock:
                self._input_size = (width, height)
                self._output_size = output_size
            resize_started = time.monotonic()
            if (width, height) != output_size:
                frame = self.resize_frame(frame, *output_size)
            resize_finished = time.monotonic()

            max_age = float(self.detection_max_age_getter())
            self.overlay.detection_max_age = max_age
            snapshot = self.detection_store.latest(max_age_seconds=max_age)
            control = self.ai_vision_control.latest()
            overlay_started = time.monotonic()
            annotated, _ = self.overlay.render(frame, snapshot, copy_frame=False, control=control)
            overlay_finished = time.monotonic()
            published = self.publisher.push_frame(annotated)
            push_finished = time.monotonic()
            with self._metrics_lock:
                self._video_processing_frames += 1
                self._resize_time_seconds += resize_finished - resize_started
                self._overlay_time_seconds += overlay_finished - overlay_started
                self._push_time_seconds += push_finished - overlay_finished
                if published:
                    self._video_output_frames += 1
                else:
                    self._video_publish_failures += 1

    def stop(self, join_timeout: float = 5.0) -> None:
        self._stop_event.set()
        self.video_source.signal_stop()
        self.inference_source.signal_stop()
        self.video_source.join(join_timeout)
        if self._inference_thread is not None:
            self._inference_thread.join(join_timeout)
        if self._video_thread is not None:
            self._video_thread.join(join_timeout)
        workers_alive = self.is_alive
        self.inference_source.close_reader()
        self.publisher.stop_pipeline()
        if workers_alive:
            raise RuntimeError("vision worker failed to stop cleanly")

    def check_output_stale(self) -> bool:
        return self.publisher.check_stale()

    def metrics(self) -> PipelineMetrics:
        snapshot = self.detection_store.latest()
        with self._metrics_lock:
            output = self._video_output_frames
            inference = self._inference_completed
            failures = self._video_publish_failures
            rate_limited = self._video_rate_limited_frames
            processing = self._video_processing_frames
            resize_time = self._resize_time_seconds
            overlay_time = self._overlay_time_seconds
            push_time = self._push_time_seconds
            input_size, output_size = self._input_size, self._output_size
        return PipelineMetrics(
            video_input_frames=int(self.video_source.received_frames),
            video_output_frames=output,
            inference_completed=inference,
            video_publish_failures=failures,
            dropped_video_frames=int(self.video_source.dropped_frames),
            dropped_inference_frames=int(self.inference_source.dropped_frames),
            snapshot_age_seconds=None if snapshot is None else snapshot.age_seconds(),
            input_width=input_size[0], input_height=input_size[1],
            output_width=output_size[0], output_height=output_size[1],
            video_rate_limited_frames=rate_limited,
            video_processing_frames=processing,
            resize_time_seconds=resize_time,
            overlay_time_seconds=overlay_time,
            push_time_seconds=push_time,
        )

    @property
    def is_alive(self) -> bool:
        inference_alive = self._inference_thread is not None and self._inference_thread.is_alive()
        video_alive = self._video_thread is not None and self._video_thread.is_alive()
        source_alive = bool(getattr(self.video_source, "is_alive", False))
        return inference_alive or video_alive or source_alive


def _resize_frame(frame, width: int, height: int):
    import cv2

    return cv2.resize(frame, (width, height), interpolation=cv2.INTER_LINEAR)
