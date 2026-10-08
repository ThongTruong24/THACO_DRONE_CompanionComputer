"""Exercise real publisher caps, H.264/FLV encoding and decoding without a server."""
import time

import numpy as np
import pytest
from vision_v1.publisher import DEFAULT_BITRATE_KBPS, VisionPublisher, make_pipeline_desc


def test_real_gstreamer_encodes_actual_geometry_after_rebuild(monkeypatch):
    gi = pytest.importorskip("gi")
    gi.require_version("Gst", "1.0")
    from gi.repository import Gst

    Gst.init(None)
    required = ("x264enc", "flvmux", "flvdemux", "avdec_h264")
    if any(Gst.ElementFactory.find(name) is None for name in required):
        pytest.skip("H.264/FLV GStreamer plugins unavailable")

    def loopback(*args):
        return make_pipeline_desc(*args).split("rtmpsink", 1)[0] + (
            "flvdemux ! h264parse ! avdec_h264 max-threads=1 ! videoconvert ! "
            "video/x-raw,format=BGR ! appsink name=decoded sync=false max-buffers=2 drop=true"
        )

    monkeypatch.setattr("vision_v1.publisher.make_pipeline_desc", loopback)
    publisher = VisionPublisher(640, 480, 30, DEFAULT_BITRATE_KBPS, "rtmp://unused/yolo", 2)
    previous_pipeline = None
    try:
        for width, height in [(640, 360), (320, 240)]:
            frame = np.zeros((height, width, 3), dtype=np.uint8)
            frame[:, :width // 2] = (0, 255, 255)
            assert publisher.push_frame(frame)
            assert publisher.pipeline is not previous_pipeline
            previous_pipeline = publisher.pipeline
            sink = publisher.pipeline.get_by_name("decoded")
            sample = None
            deadline = time.monotonic() + 5
            while sample is None and time.monotonic() < deadline:
                assert publisher.push_frame(frame)
                sample = sink.emit("try-pull-sample", 100 * Gst.MSECOND)
            assert sample is not None
            caps = sample.get_caps().get_structure(0)
            assert caps.get_value("width") == width
            assert caps.get_value("height") == height
            assert sample.get_buffer().pts >= 0
            assert not publisher._bus_failed()
    finally:
        publisher.stop_pipeline()
