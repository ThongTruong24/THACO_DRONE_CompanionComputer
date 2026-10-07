import sys
from types import SimpleNamespace

from vision_v1.detection_types import Detection, DetectionSnapshot
from vision_v1.overlay import OverlayRenderer, scale_bbox
from vision_v1.ai_vision_control import AiVisionControlState, AiVisionControlStore


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
    _, used = renderer.render(
        FakeFrame(), snapshot, now_ns=1_100_000_000, control=AiVisionControlState(bounding_box=True)
    )
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
    _, used = OverlayRenderer(0.2).render(
        FakeFrame(), snapshot, now_ns=2_000_000_000, control=AiVisionControlState(bounding_box=True)
    )
    assert not used


def draw_calls(monkeypatch):
    calls = []
    monkeypatch.setitem(sys.modules, "cv2", SimpleNamespace(
        FONT_HERSHEY_SIMPLEX=0, LINE_AA=0,
        rectangle=lambda frame, p1, p2, color, thickness: calls.append(("rect", p1, p2, color, thickness)),
        putText=lambda frame, text, origin, font, size, color, *args: calls.append(("text", text, color)),
    ))
    return calls


def make_snapshot(detections):
    return DetectionSnapshot.create(
        source_timestamp_us=1, source_width=640, source_height=480,
        generation=1, source_seq=2, detections=detections,
        completed_monotonic_ns=1_000_000_000,
    )


def test_bbox_off_suppresses_all_boxes_even_selected(monkeypatch):
    calls = draw_calls(monkeypatch)
    snapshot = make_snapshot([Detection(10, 20, 110, 220, 1, "plant", 0.8, track_id=7)])
    _, used = OverlayRenderer().render(
        FakeFrame(), snapshot, now_ns=1_100_000_000,
        control=AiVisionControlState(False, True, True, selected_track_id=7),
    )
    assert not used
    assert calls == []


def test_tracking_off_draws_normal_boxes_without_selected_highlight(monkeypatch):
    calls = draw_calls(monkeypatch)
    snapshot = make_snapshot([Detection(10, 20, 110, 220, 1, "plant", 0.8, track_id=7)])
    OverlayRenderer().render(
        FakeFrame(), snapshot, now_ns=1_100_000_000,
        control=AiVisionControlState(True, False, True, selected_track_id=7),
    )
    assert calls[0][3:] == ((0, 255, 0), 2)
    assert "SELECTED" not in calls[1][1]


def test_target_highlight_follows_id_through_motion_loss_and_reappearance(monkeypatch):
    calls = draw_calls(monkeypatch)
    store = AiVisionControlStore()
    store.update(True, True, False)
    original = Detection(10, 20, 110, 220, 1, "plant", 0.8, track_id=7)
    first = make_snapshot([original])
    assert store.select_track_point(60 / 640, 120 / 480, 0, first, max_age_seconds=0.5, now_ns=1_100_000_000)
    other = Detection(100, 20, 160, 100, 1, "plant", 0.8, track_id=8)
    moved = Detection(20, 40, 120, 240, 1, "plant", 0.8, track_id=7)
    frames = ([original], [other, moved], [other], [moved, other])
    renderer = OverlayRenderer()
    for index, detections in enumerate(frames):
        calls.clear()
        renderer.render(FakeFrame(), make_snapshot(detections), now_ns=1_100_000_000, control=store.latest())
        highlighted = [call for call in calls if call[0] == "rect" and call[3] == (0, 165, 255)]
        assert len(highlighted) == (0 if index == 2 else 1)
        if index == 1:
            assert highlighted[0][1:3] == ((10, 20), (60, 120))
        assert store.latest().selected_track_id == 7


def test_detection_without_id_still_renders_normally(monkeypatch):
    calls = draw_calls(monkeypatch)
    OverlayRenderer().render(
        FakeFrame(), make_snapshot([Detection(10, 20, 110, 220, 1, "plant", 0.8)]),
        now_ns=1_100_000_000, control=AiVisionControlState(True, True, True),
    )
    assert calls[0][3:] == ((0, 255, 0), 2)
