from types import SimpleNamespace

import numpy as np
import pytest
from vision_v1.publisher import (
    DEFAULT_BITRATE_KBPS, DEFAULT_ENCODER_PRESET, DEFAULT_KEY_INT_MAX,
    VisionPublisher, make_pipeline_desc,
)


def test_publisher_pipeline_is_bounded_and_low_latency():
    pipeline = make_pipeline_desc(640, 480, 30, DEFAULT_BITRATE_KBPS, "rtmp://127.0.0.1:1935/yolo")
    assert "block=false" in pipeline
    assert "max-buffers=2" in pipeline
    assert "max-bytes=0" in pipeline.split(" ! ")[0]
    assert "leaky-type=downstream" in pipeline
    assert "leaky=downstream" in pipeline
    assert "tune=zerolatency" in pipeline
    assert "bitrate=2000" in pipeline
    assert "speed-preset=ultrafast" in pipeline
    assert "key-int-max=30" in pipeline
    assert "threads=1" in pipeline
    assert "bframes=0" in pipeline
    assert "byte-stream=false" in pipeline
    assert "profile=baseline" in pipeline
    assert "rtmp://127.0.0.1:1935/yolo" in pipeline


def test_publisher_fps_reconfiguration_updates_timing():
    publisher = VisionPublisher(640, 480, 30, 1000, "rtmp://127.0.0.1:1935/yolo", 2.0)
    publisher.set_fps(20)
    assert publisher.fps == 20


def fake_gst(monkeypatch):
    monkeypatch.setattr(VisionPublisher, "_init_gst", lambda self: None)
    publisher = VisionPublisher(640, 480, 30, 1000, "rtmp://example/yolo", 2.0)
    pipelines = []

    class Buffer:
        @staticmethod
        def new_allocate(*args):
            return Buffer()

        def fill(self, offset, data):
            self.data = data

    class Source:
        def __init__(self):
            self.buffers = []

        def set_property(self, name, value):
            self.caps = value

        def emit(self, event, buffer):
            self.buffers.append(buffer)
            return "ok"

    class Pipeline:
        def __init__(self, description):
            self.description = description
            self.source = Source()
            self.states = []
            pipelines.append(self)

        def get_by_name(self, name):
            return self.source

        def set_state(self, state):
            self.states.append(state)
            return "success"

        def get_bus(self):
            return SimpleNamespace(timed_pop_filtered=lambda *args: None)

    publisher.Gst = SimpleNamespace(
        parse_launch=Pipeline, Caps=SimpleNamespace(from_string=lambda caps: caps),
        Buffer=Buffer, SECOND=1_000_000_000,
        State=SimpleNamespace(PLAYING="playing", NULL="null"),
        StateChangeReturn=SimpleNamespace(FAILURE="failure"),
        FlowReturn=SimpleNamespace(OK="ok"), MessageType=SimpleNamespace(ERROR=1, EOS=2),
    )
    publisher._gst_available = True
    return publisher, pipelines


def test_caps_follow_actual_frames_and_rebuild_only_on_size_change(monkeypatch):
    publisher, pipelines = fake_gst(monkeypatch)
    for shape in [(360, 640, 3), (360, 640, 3), (240, 320, 3)]:
        assert publisher.push_frame(np.zeros(shape, dtype=np.uint8))
    assert len(pipelines) == 2
    assert pipelines[0].states[-1] == "null"
    assert "width=640,height=360" in pipelines[0].source.caps
    assert "width=320,height=240" in pipelines[1].source.caps
    assert "pixel-aspect-ratio=1/1" in pipelines[1].source.caps
    for pipeline in pipelines:
        for buffer in pipeline.source.buffers:
            assert buffer.pts >= 0
            assert buffer.duration == 1_000_000_000 // 30


def test_first_frame_and_reconfiguration_timestamp_follow_new_epoch(monkeypatch):
    publisher, pipelines = fake_gst(monkeypatch)
    ticks = iter([10.0, 10.1, 20.0, 20.1])
    monkeypatch.setattr("vision_v1.publisher.time.monotonic", lambda: next(ticks))
    assert publisher.push_frame(np.zeros((360, 640, 3), dtype=np.uint8))
    assert publisher.push_frame(np.zeros((240, 320, 3), dtype=np.uint8))
    assert all(p.source.buffers[0].pts > 0 for p in pipelines)


def test_invalid_frame_does_not_reconfigure_pipeline(monkeypatch):
    publisher, pipelines = fake_gst(monkeypatch)
    assert not publisher.push_frame(np.zeros((360, 641, 3), dtype=np.uint8))
    assert not publisher.push_frame(np.zeros((360, 640, 3), dtype=np.float32))
    assert not publisher.push_frame(None)
    assert not publisher.push_frame(np.zeros((360, 640), dtype=np.uint8))
    assert not publisher.push_frame(np.zeros((361, 640, 3), dtype=np.uint8))
    assert not pipelines


@pytest.mark.parametrize("preset", ["ultrafast", "superfast", "veryfast"])
def test_configured_encoder_settings_reach_real_publisher_pipeline(monkeypatch, preset):
    publisher, pipelines = fake_gst(monkeypatch)
    publisher.encoder_preset = preset
    publisher.key_int_max = 45
    assert publisher.push_frame(np.zeros((360, 640, 3), dtype=np.uint8))
    assert f"speed-preset={preset}" in pipelines[0].description
    assert "key-int-max=45" in pipelines[0].description
    publisher.set_fps(20)
    assert publisher.push_frame(np.zeros((360, 640, 3), dtype=np.uint8))
    assert "key-int-max=45" in pipelines[1].description
    assert "framerate=20/1" in pipelines[1].source.caps


@pytest.mark.parametrize("preset", ["", "medium", "ULTRAFAST", "ultrafast ! fakesink", None])
def test_invalid_encoder_preset_is_rejected_before_gst_init(monkeypatch, preset):
    monkeypatch.setattr(VisionPublisher, "_init_gst", lambda self: pytest.fail("GStreamer initialized"))
    with pytest.raises(ValueError, match="encoder_preset"):
        VisionPublisher(640, 360, 30, DEFAULT_BITRATE_KBPS, "rtmp://example/yolo", 2, encoder_preset=preset)
    with pytest.raises(ValueError, match="encoder_preset"):
        make_pipeline_desc(640, 360, 30, DEFAULT_BITRATE_KBPS, "rtmp://example/yolo", preset)


@pytest.mark.parametrize("gop", [0, -1, 1.5, True])
def test_invalid_gop_is_rejected(gop):
    with pytest.raises(ValueError, match="positive integer"):
        make_pipeline_desc(640, 360, 30, DEFAULT_BITRATE_KBPS, "rtmp://example/yolo", key_int_max=gop)


def test_default_gop_is_independent_of_fps(monkeypatch):
    publisher, pipelines = fake_gst(monkeypatch)
    assert publisher.encoder_preset == DEFAULT_ENCODER_PRESET
    assert publisher.key_int_max == DEFAULT_KEY_INT_MAX
    publisher.set_fps(15)
    assert publisher.push_frame(np.zeros((360, 640, 3), dtype=np.uint8))
    assert "key-int-max=30" in pipelines[0].description


def test_active_caps_mismatch_is_rejected_without_pushing(monkeypatch):
    publisher, pipelines = fake_gst(monkeypatch)
    frame = np.zeros((360, 640, 3), dtype=np.uint8)
    assert publisher.push_frame(frame)
    publisher._caps_size = (320, 240)
    assert not publisher.push_frame(frame)
    assert len(pipelines[0].source.buffers) == 1


def test_bus_error_rebuilds_publisher_with_same_settings(monkeypatch):
    publisher, pipelines = fake_gst(monkeypatch)
    frame = np.zeros((360, 640, 3), dtype=np.uint8)
    assert publisher.push_frame(frame)
    publisher.bus = SimpleNamespace(timed_pop_filtered=lambda *args: object())
    assert publisher.push_frame(frame)
    assert len(pipelines) == 2
    assert pipelines[0].states[-1] == "null"
    assert pipelines[0].description == pipelines[1].description
