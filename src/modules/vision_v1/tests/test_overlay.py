import sys
from types import SimpleNamespace

from vision_v1.detection_types import Detection, DetectionSnapshot
from vision_v1.overlay import OverlayRenderer, scale_bbox


class FakeFrame:
    shape = (240, 320, 3)

    def copy(self):
        return FakeFrame()


def test_coordinate_scaling():
    detection = Detection(10, 20, 110, 220, 1, "plant", 0.8)
    assert scale_bbox(detection, 640, 480, 320, 240) == (5.0, 10.0, 55.0, 110.0)


def test_overlay_draws_scaled_label_and_distance(monkeypatch):
    calls = []
    fake_cv2 = SimpleNamespace(
        FONT_HERSHEY_SIMPLEX=0,
        LINE_AA=0,
        rectangle=lambda frame, p1, p2, color, thickness: calls.append(("rect", p1, p2)),
        putText=lambda frame, label, origin, *args: calls.append(("text", label, origin)),
    )
    monkeypatch.setitem(sys.modules, "cv2", fake_cv2)
    snapshot = DetectionSnapshot.create(
        source_timestamp_us=1,
        source_width=640,
        source_height=480,
        generation=1,
        source_seq=2,
        detections=(Detection(10, 20, 110, 220, 1, "plant", 0.8, 2.5),),
        completed_monotonic_ns=1_000_000_000,
    )
    renderer = OverlayRenderer(0.5)
    _, used = renderer.render(FakeFrame(), snapshot, now_ns=1_100_000_000)
    assert used
    assert ("rect", (5, 10), (55, 110)) in calls
    assert any("plant 0.80 2.50m" in call[1] for call in calls if call[0] == "text")


def test_overlay_ignores_stale_snapshot(monkeypatch):
    monkeypatch.setitem(sys.modules, "cv2", SimpleNamespace())
    snapshot = DetectionSnapshot.create(
        source_timestamp_us=1,
        source_width=640,
        source_height=480,
        generation=1,
        source_seq=2,
        detections=(),
        completed_monotonic_ns=1_000_000_000,
    )
    _, used = OverlayRenderer(0.2).render(FakeFrame(), snapshot, now_ns=2_000_000_000)
    assert not used
