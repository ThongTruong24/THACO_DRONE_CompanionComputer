import time

from vision_v1.detection_store import DetectionStore
from vision_v1.detection_types import DetectionSnapshot


def snapshot(completed_ns):
    return DetectionSnapshot.create(
        source_timestamp_us=10,
        source_width=640,
        source_height=480,
        generation=1,
        source_seq=2,
        detections=(),
        completed_monotonic_ns=completed_ns,
    )


def test_store_keeps_latest_snapshot():
    store = DetectionStore()
    first = snapshot(100)
    second = snapshot(200)
    store.update(first)
    store.update(second)
    assert store.latest() is second


def test_store_rejects_stale_snapshot():
    now = time.monotonic_ns()
    store = DetectionStore()
    store.update(snapshot(now - 2_000_000_000))
    assert store.latest(max_age_seconds=0.5, now_ns=now) is None
