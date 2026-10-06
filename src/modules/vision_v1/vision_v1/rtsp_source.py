"""Reconnectable low-latency GStreamer RTSP source."""
from __future__ import annotations

import logging
import threading
from typing import Callable, Optional

from .latest_value import LatestValue


LOG = logging.getLogger("vision_v1.rtsp_source")


def quote(value: str) -> str:
    return '"' + str(value).replace("\\", "\\\\").replace('"', '\\"') + '"'


def make_pipeline_desc(input_url: str, latency_ms: int = 80) -> str:
    return (
        f"rtspsrc location={quote(input_url)} protocols=tcp latency={int(latency_ms)} "
        "drop-on-latency=true ! rtph264depay ! h264parse ! avdec_h264 ! "
        "videoconvert ! video/x-raw,format=BGR ! "
        "appsink name=video_sink emit-signals=true sync=false max-buffers=1 drop=true"
    )


class GStreamerRtspBackend:
    def __init__(self, input_url: str, latency_ms: int = 80):
        self.input_url = input_url
        self.latency_ms = latency_ms
        self._pipeline = None
        self._lock = threading.Lock()

    def run(self, on_frame: Callable[[object], None], stop_event: threading.Event) -> None:
        import gi

        gi.require_version("Gst", "1.0")
        from gi.repository import Gst
        import numpy as np

        Gst.init(None)
        pipeline = Gst.parse_launch(make_pipeline_desc(self.input_url, self.latency_ms))
        appsink = pipeline.get_by_name("video_sink")
        if appsink is None:
            pipeline.set_state(Gst.State.NULL)
            raise RuntimeError("GStreamer pipeline has no video_sink appsink")

        def on_sample(sink):
            sample = sink.emit("pull-sample")
            if sample is None:
                return Gst.FlowReturn.ERROR
            buffer = sample.get_buffer()
            caps = sample.get_caps()
            structure = caps.get_structure(0)
            width = structure.get_value("width")
            height = structure.get_value("height")
            ok, mapped = buffer.map(Gst.MapFlags.READ)
            if not ok:
                return Gst.FlowReturn.ERROR
            try:
                frame = np.frombuffer(mapped.data, dtype=np.uint8).reshape((height, width, 3)).copy()
            finally:
                buffer.unmap(mapped)
            on_frame(frame)
            return Gst.FlowReturn.OK

        appsink.connect("new-sample", on_sample)
        with self._lock:
            self._pipeline = pipeline
        try:
            if pipeline.set_state(Gst.State.PLAYING) == Gst.StateChangeReturn.FAILURE:
                raise RuntimeError("RTSP pipeline failed to enter PLAYING")
            bus = pipeline.get_bus()
            while not stop_event.is_set():
                message = bus.timed_pop_filtered(
                    100 * Gst.MSECOND,
                    Gst.MessageType.ERROR | Gst.MessageType.EOS,
                )
                if message is None:
                    continue
                if message.type == Gst.MessageType.ERROR:
                    error, debug = message.parse_error()
                    raise RuntimeError(f"RTSP GStreamer error: {error}; {debug}")
                raise RuntimeError("RTSP GStreamer EOS")
        finally:
            pipeline.set_state(Gst.State.NULL)
            with self._lock:
                self._pipeline = None

    def request_stop(self) -> None:
        with self._lock:
            pipeline = self._pipeline
        if pipeline is not None:
            try:
                from gi.repository import Gst

                pipeline.set_state(Gst.State.NULL)
            except Exception:
                pass


class RtspVideoSource:
    def __init__(
        self,
        input_url: str,
        reconnect_seconds: float = 1.0,
        latency_ms: int = 80,
        backend_factory=None,
    ):
        if reconnect_seconds <= 0:
            raise ValueError("reconnect_seconds must be positive")
        self.input_url = input_url
        self.reconnect_seconds = float(reconnect_seconds)
        self.latency_ms = int(latency_ms)
        self.backend_factory = backend_factory or (
            lambda: GStreamerRtspBackend(self.input_url, self.latency_ms)
        )
        self.mailbox = LatestValue()
        self.received_frames = 0
        self.reconnect_count = 0
        self.last_error = ""
        self._stop_event = threading.Event()
        self._thread: Optional[threading.Thread] = None
        self._backend = None
        self._backend_lock = threading.Lock()

    def start(self) -> None:
        if self._thread is not None and self._thread.is_alive():
            return
        self._stop_event.clear()
        self._thread = threading.Thread(target=self._run, name="rtsp-source", daemon=False)
        self._thread.start()

    def _run(self) -> None:
        while not self._stop_event.is_set():
            backend = self.backend_factory()
            with self._backend_lock:
                self._backend = backend
            try:
                backend.run(self._on_frame, self._stop_event)
                if not self._stop_event.is_set():
                    self.last_error = "RTSP backend stopped unexpectedly"
            except Exception as exc:
                if not self._stop_event.is_set():
                    self.last_error = str(exc)
                    LOG.warning("RTSP source disconnected: %s", exc)
            finally:
                with self._backend_lock:
                    self._backend = None

            if self._stop_event.is_set():
                break
            self.reconnect_count += 1
            self._stop_event.wait(self.reconnect_seconds)
        self.mailbox.close()

    def _on_frame(self, frame) -> None:
        if self.mailbox.put(frame):
            self.received_frames += 1

    def wait_next(self, last_version: int, timeout: Optional[float] = None):
        return self.mailbox.wait_next(last_version, timeout)

    def signal_stop(self) -> None:
        self._stop_event.set()
        with self._backend_lock:
            backend = self._backend
        if backend is not None:
            backend.request_stop()
        self.mailbox.close()

    def join(self, timeout: Optional[float] = None) -> None:
        if self._thread is not None:
            self._thread.join(timeout)

    @property
    def is_alive(self) -> bool:
        return self._thread is not None and self._thread.is_alive()

    @property
    def dropped_frames(self) -> int:
        return self.mailbox.dropped
