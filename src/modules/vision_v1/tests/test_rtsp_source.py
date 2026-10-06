import time

from vision_v1.rtsp_source import RtspVideoSource, make_pipeline_desc


def wait_until(predicate, timeout=1.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.01)
    return predicate()


def test_rtsp_pipeline_uses_drop_old_appsink():
    pipeline = make_pipeline_desc("rtsp://127.0.0.1:8554/camera")
    assert "protocols=tcp" in pipeline
    assert "drop-on-latency=true" in pipeline
    assert "max-buffers=1" in pipeline
    assert "drop=true" in pipeline


def test_rtsp_source_keeps_latest_frame_and_counts_drops():
    source = RtspVideoSource("rtsp://example/camera")
    source._on_frame("one")
    source._on_frame("two")
    source._on_frame("three")
    version, frame = source.wait_next(0, timeout=0)
    assert version == 3
    assert frame == "three"
    assert source.dropped_frames == 2


def test_rtsp_source_reconnects_after_backend_failure():
    created = []

    class Backend:
        def __init__(self, fail):
            self.fail = fail

        def run(self, on_frame, stop_event):
            if self.fail:
                raise RuntimeError("disconnect")
            stop_event.wait(1.0)

        def request_stop(self):
            pass

    def factory():
        backend = Backend(fail=not created)
        created.append(backend)
        return backend

    source = RtspVideoSource(
        "rtsp://example/camera",
        reconnect_seconds=0.01,
        backend_factory=factory,
    )
    source.start()
    assert wait_until(lambda: source.reconnect_count >= 1 and len(created) >= 2)
    source.signal_stop()
    source.join(1.0)
    assert not source.is_alive
