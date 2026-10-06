"""GStreamer RTMP Publisher for annotated vision video stream."""
import logging
import time
import numpy as np

LOG = logging.getLogger("vision.publisher")


def quote(value):
    return '"' + str(value).replace('\\', '\\\\').replace('"', '\\"') + '"'


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
    """
    Manages GStreamer pipeline for streaming annotated frames to MediaMTX.
    If no new frame arrives within stale_seconds, stops pipeline (GST_STATE_NULL)
    and reports is_idle = True.
    """
    def __init__(self, width: int = 640, height: int = 480, fps: int = 15,
                 bitrate_kbps: int = 1000, output_url: str = "rtmp://127.0.0.1:1935/yolo",
                 stale_seconds: float = 2.0):
        self.width = width
        self.height = height
        self.fps = fps
        self.bitrate_kbps = bitrate_kbps
        self.output_url = output_url
        self.stale_seconds = stale_seconds

        self._gst_available = False
        self._init_gst()

        self.pipeline = None
        self.appsrc = None
        self.bus = None
        self.epoch = None
        self.is_idle = True
        self.published_frames = 0
        self.last_frame_time = 0.0

    def _init_gst(self):
        try:
            import gi
            gi.require_version("Gst", "1.0")
            from gi.repository import Gst
            Gst.init(None)
            self.Gst = Gst
            self._gst_available = True
        except Exception as e:
            LOG.warning(f"GStreamer python bindings not available: {e}")
            self._gst_available = False

    def start_pipeline(self) -> bool:
        if not self._gst_available:
            return False
        self.stop_pipeline()
        try:
            desc = make_pipeline_desc(self.width, self.height, self.fps, self.bitrate_kbps, self.output_url)
            self.pipeline = self.Gst.parse_launch(desc)
            self.appsrc = self.pipeline.get_by_name("frames")
            caps_str = f"video/x-raw,format=BGR,width={self.width},height={self.height},framerate={self.fps}/1"
            self.appsrc.set_property("caps", self.Gst.Caps.from_string(caps_str))
            ret = self.pipeline.set_state(self.Gst.State.PLAYING)
            if ret == self.Gst.StateChangeReturn.FAILURE:
                self.stop_pipeline()
                return False
            self.bus = self.pipeline.get_bus()
            self.epoch = time.monotonic()
            self.is_idle = False
            LOG.info(f"Vision GStreamer pipeline started -> {self.output_url}")
            return True
        except Exception as e:
            LOG.error(f"Failed to start pipeline: {e}")
            self.stop_pipeline()
            return False

    def stop_pipeline(self):
        if self.pipeline is not None and self._gst_available:
            try:
                self.pipeline.set_state(self.Gst.State.NULL)
            except Exception:
                pass
            self.pipeline = None
            self.appsrc = None
            self.bus = None
        self.is_idle = True

    def check_bus_error(self) -> bool:
        """Returns True if an error occurred on the bus."""
        if not self._gst_available or self.bus is None:
            return False
        msg = self.bus.timed_pop_filtered(0, self.Gst.MessageType.ERROR | self.Gst.MessageType.EOS)
        if msg:
            err_str = str(msg.parse_error()) if msg.type == self.Gst.MessageType.ERROR else "EOS"
            LOG.warning(f"GStreamer bus message: {err_str}")
            return True
        return False

    def push_frame(self, frame_bgr: np.ndarray, timestamp: float = 0.0) -> bool:
        """Push a newly annotated frame to GStreamer."""
        now = time.monotonic()
        self.last_frame_time = now

        if self.is_idle or self.pipeline is None:
            if not self.start_pipeline():
                return False

        if self.check_bus_error():
            LOG.warning("GStreamer bus error detected; restarting pipeline...")
            if not self.start_pipeline():
                return False

        if not self._gst_available or self.appsrc is None:
            return False

        try:
            data = frame_bgr.tobytes()
            buf = self.Gst.Buffer.new_allocate(None, len(data), None)
            buf.fill(0, data)
            pts = int((now - self.epoch) * self.Gst.SECOND)
            buf.pts = pts
            buf.dts = pts
            buf.duration = self.Gst.SECOND // self.fps

            ret = self.appsrc.emit("push-buffer", buf)
            if ret != self.Gst.FlowReturn.OK:
                LOG.warning("appsrc push-buffer failed")
                return False
            self.published_frames += 1
            return True
        except Exception as e:
            LOG.error(f"push_frame error: {e}")
            return False

    def check_stale(self) -> bool:
        """
        Check if stream is stale (> stale_seconds without new frame).
        If stale, sets pipeline to NULL and marks is_idle = True.
        """
        if self.is_idle:
            return True

        if time.monotonic() - self.last_frame_time > self.stale_seconds:
            LOG.info("Vision stream is stale -> setting pipeline to NULL (idle)")
            self.stop_pipeline()
            self.is_idle = True
            return True
        return False
