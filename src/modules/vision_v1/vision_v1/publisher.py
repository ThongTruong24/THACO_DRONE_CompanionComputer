"""Bounded GStreamer RTMP publisher for annotated video."""
from __future__ import annotations

import logging
import threading
import time


LOG = logging.getLogger("vision_v1.publisher")


def quote(value: str) -> str:
    return '"' + str(value).replace("\\", "\\\\").replace('"', '\\"') + '"'


def make_pipeline_desc(width: int, height: int, fps: int, bitrate_kbps: int, output_url: str) -> str:
    return (
        "appsrc name=frames is-live=true format=time block=false max-buffers=2 "
        "leaky-type=downstream ! queue max-size-buffers=2 max-size-time=0 "
        "max-size-bytes=0 leaky=downstream ! videoconvert ! video/x-raw,format=I420 ! "
        f"x264enc tune=zerolatency speed-preset=ultrafast threads=1 bitrate={bitrate_kbps} "
        f"key-int-max={fps} bframes=0 byte-stream=false ! video/x-h264,profile=baseline ! "
        "h264parse config-interval=-1 ! flvmux streamable=true ! "
        f"rtmpsink location={quote(output_url)} sync=false async=false"
    )


class VisionPublisher:
    def __init__(
        self,
        width: int,
        height: int,
        fps: int,
        bitrate_kbps: int,
        output_url: str,
        stale_seconds: float,
    ):
        self.width = int(width)
        self.height = int(height)
        self.fps = int(fps)
        self.bitrate_kbps = int(bitrate_kbps)
        self.output_url = output_url
        self.stale_seconds = float(stale_seconds)
        self.pipeline = None
        self.appsrc = None
        self.bus = None
        self.epoch = 0.0
        self.last_frame_time = 0.0
        self.published_frames = 0
        self.is_idle = True
        self._lock = threading.RLock()
        self._gst_available = False
        self._init_gst()

    def _init_gst(self) -> None:
        try:
            import gi

            gi.require_version("Gst", "1.0")
            from gi.repository import Gst

            Gst.init(None)
            self.Gst = Gst
            self._gst_available = True
        except Exception as exc:
            LOG.warning("GStreamer Python bindings unavailable: %s", exc)

    def start_pipeline(self) -> bool:
        with self._lock:
            if not self._gst_available:
                return False
            self.stop_pipeline()
            try:
                self.pipeline = self.Gst.parse_launch(
                    make_pipeline_desc(
                        self.width,
                        self.height,
                        self.fps,
                        self.bitrate_kbps,
                        self.output_url,
                    )
                )
                self.appsrc = self.pipeline.get_by_name("frames")
                caps = self.Gst.Caps.from_string(
                    f"video/x-raw,format=BGR,width={self.width},height={self.height},framerate={self.fps}/1"
                )
                self.appsrc.set_property("caps", caps)
                if self.pipeline.set_state(self.Gst.State.PLAYING) == self.Gst.StateChangeReturn.FAILURE:
                    self.stop_pipeline()
                    return False
                self.bus = self.pipeline.get_bus()
                self.epoch = time.monotonic()
                self.is_idle = False
                return True
            except Exception as exc:
                LOG.error("Failed to start output pipeline: %s", exc)
                self.stop_pipeline()
                return False

    def push_frame(self, frame_bgr, timestamp: float = 0.0) -> bool:
        del timestamp
        with self._lock:
            now = time.monotonic()
            if self.is_idle or self.pipeline is None:
                if not self.start_pipeline():
                    return False
            if self._bus_failed() and not self.start_pipeline():
                return False
            if self.appsrc is None:
                return False
            try:
                data = frame_bgr.tobytes()
                buffer = self.Gst.Buffer.new_allocate(None, len(data), None)
                buffer.fill(0, data)
                pts = int((now - self.epoch) * self.Gst.SECOND)
                buffer.pts = pts
                buffer.dts = pts
                buffer.duration = self.Gst.SECOND // max(1, self.fps)
                if self.appsrc.emit("push-buffer", buffer) != self.Gst.FlowReturn.OK:
                    return False
                self.last_frame_time = now
                self.published_frames += 1
                return True
            except Exception as exc:
                LOG.error("Output push failed: %s", exc)
                return False

    def set_fps(self, fps: int) -> None:
        value = int(fps)
        if value <= 0:
            raise ValueError("fps must be positive")
        with self._lock:
            if value == self.fps:
                return
            self.fps = value
            self.stop_pipeline()

    def _bus_failed(self) -> bool:
        if self.bus is None:
            return False
        message = self.bus.timed_pop_filtered(
            0,
            self.Gst.MessageType.ERROR | self.Gst.MessageType.EOS,
        )
        return message is not None

    def check_stale(self) -> bool:
        with self._lock:
            if self.is_idle:
                return True
            if time.monotonic() - self.last_frame_time > self.stale_seconds:
                self.stop_pipeline()
                return True
            return False

    def stop_pipeline(self) -> None:
        with self._lock:
            if self.pipeline is not None and self._gst_available:
                try:
                    self.pipeline.set_state(self.Gst.State.NULL)
                except Exception:
                    pass
            self.pipeline = None
            self.appsrc = None
            self.bus = None
            self.is_idle = True
