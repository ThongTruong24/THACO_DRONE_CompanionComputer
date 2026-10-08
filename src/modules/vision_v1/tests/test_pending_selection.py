import threading

import pytest
from vision_v1.ai_vision_control import AiVisionControlStore
from vision_v1.detection_types import Detection, DetectionSnapshot


class Clock:
    ns = 1_000_000_000

    def __call__(self):
        return self.ns

    def advance(self, seconds):
        self.ns += round(seconds * 1_000_000_000)


@pytest.fixture
def context():
    clock = Clock()
    store = AiVisionControlStore(clock_ns=clock)
    store.update(True, True, False)
    return store, clock


def snapshot(clock, ids=(), center=(200, 200)):
    return DetectionSnapshot.create(
        source_timestamp_us=clock.ns // 1000, source_width=640, source_height=480,
        generation=1, source_seq=clock.ns,
        detections=[Detection(center[0] - 100, center[1] - 100,
                              center[0] + 100, center[1] + 100,
                              0, "plant", 0.9, track_id=track_id) for track_id in ids],
        completed_monotonic_ns=clock.ns,
    )


def click(store, value=None, x=110 / 640, y=110 / 480):
    return store.select_track_point(x, y, 0, value, max_age_seconds=0.5)


def infer(store, value):
    return store.on_detection_snapshot(value, max_age_seconds=0.5)


def test_pending_matches_at_point_seven_seconds(context):
    store, clock = context
    assert not click(store, snapshot(clock))
    clock.advance(0.3)
    assert not infer(store, snapshot(clock))
    clock.advance(0.4)
    assert infer(store, snapshot(clock, [7]))
    assert store.latest().selected_track_id == 7
    assert store.latest().pending_click is None
    assert not infer(store, snapshot(clock, [7]))  # Event consumed; never toggles twice.
    assert store.latest().selected_track_id == 7


@pytest.mark.parametrize("elapsed", [1.0, 1.01])
def test_pending_expires_and_late_detection_cannot_select(context, elapsed):
    store, clock = context
    click(store)
    clock.advance(elapsed)
    assert not infer(store, snapshot(clock, [7]))
    assert store.latest().pending_click is None
    assert store.latest().selected_track_id is None


def test_idle_pending_expiry_keeps_existing_target(context):
    store, clock = context
    assert click(store, snapshot(clock, [7]))
    click(store, snapshot(clock))
    clock.advance(1.01)
    assert store.latest().pending_click is None
    assert store.latest().selected_track_id == 7


def test_second_click_toggles_once(context):
    store, clock = context
    assert click(store, snapshot(clock, [7]))
    assert click(store, snapshot(clock, [7]))
    assert store.latest().selected_track_id is None
    assert store.latest().pending_click is None
    assert not infer(store, snapshot(clock, [7]))
    assert store.latest().selected_track_id is None


@pytest.mark.parametrize("returned_id,expected", [(7, None), (8, 8)])
def test_pending_deselect_or_switch(context, returned_id, expected):
    store, clock = context
    click(store, snapshot(clock, [7]))
    assert not click(store, snapshot(clock))
    assert store.latest().selected_track_id == 7
    clock.advance(0.7)
    assert infer(store, snapshot(clock, [returned_id]))
    assert store.latest().selected_track_id == expected
    assert store.latest().pending_click is None


def test_motion_missing_target_reorder_and_repeated_control_keep_id(context):
    store, clock = context
    click(store, snapshot(clock, [7]))
    for ids, center in [([8, 7], (500, 350)), ([], (500, 350)), ([7, 8], (300, 250))]:
        clock.advance(0.1)
        assert not infer(store, snapshot(clock, ids, center))
        store.update(True, True, False)
        store.update(True, True, True)
        assert store.latest().selected_track_id == 7
        assert store.latest().pending_click is None


@pytest.mark.parametrize("flags", [(True, False, True), (False, True, True)])
def test_off_clears_selected_and_pending_and_on_cannot_resurrect(context, flags):
    store, clock = context
    click(store, snapshot(clock, [7]))
    click(store, snapshot(clock))
    assert store.latest().pending_click is not None
    store.update(*flags)
    assert store.latest().selected_track_id is None
    assert store.latest().pending_click is None
    store.update(True, True, False)
    assert not infer(store, snapshot(clock, [7]))


def test_newest_click_supersedes_old_pending(context):
    store, clock = context
    click(store)
    clock.advance(0.4)
    click(store, x=500 / 640, y=350 / 480)
    assert not infer(store, snapshot(clock, [7]))
    clock.advance(0.7)
    assert infer(store, snapshot(clock, [8], (500, 350)))
    assert store.latest().selected_track_id == 8


def test_repeated_flags_and_following_preserve_pending_deadline(context):
    store, clock = context
    click(store)
    pending = store.latest().pending_click
    clock.advance(0.7)
    store.update(True, True, False)
    store.update(True, True, True)
    assert store.latest().pending_click == pending
    clock.advance(0.3)
    assert not infer(store, snapshot(clock, [7]))


def test_concurrent_snapshot_and_callback_consume_event_only_once(context, monkeypatch):
    store, clock = context
    click(store)
    entered = threading.Barrier(2)
    original = __import__("math").hypot

    def pause(*args):
        entered.wait(timeout=2)
        return original(*args)

    monkeypatch.setattr("vision_v1.ai_vision_control.math.hypot", pause)
    results = []
    threads = [threading.Thread(target=lambda: results.append(infer(store, snapshot(clock, [7]))))
               for _ in range(2)]
    for worker in threads:
        worker.start()
    for worker in threads:
        worker.join(timeout=3)
        assert not worker.is_alive()
    assert sorted(results) == [False, True]
    assert store.latest().selected_track_id == 7


def test_control_off_on_while_pending_match_runs_invalidates_event(context, monkeypatch):
    store, clock = context
    click(store)

    def off_on(*args):
        store.update(True, False, False)
        store.update(True, True, False)
        return 0.0

    monkeypatch.setattr("vision_v1.ai_vision_control.math.hypot", off_on)
    assert not infer(store, snapshot(clock, [7]))
    assert store.latest().selected_track_id is None
    assert store.latest().pending_click is None


def test_expiry_during_geometry_does_not_select(context, monkeypatch):
    store, clock = context
    click(store)

    def expire(*args):
        clock.advance(1.0)
        return 0.0

    monkeypatch.setattr("vision_v1.ai_vision_control.math.hypot", expire)
    assert not infer(store, snapshot(clock, [7]))
    assert store.latest().pending_click is None


def test_new_click_during_geometry_cannot_commit_old_target(context, monkeypatch):
    store, clock = context
    click(store)

    def newer_click(*args):
        click(store, x=500 / 640, y=350 / 480)
        return 0.0

    monkeypatch.setattr("vision_v1.ai_vision_control.math.hypot", newer_click)
    assert not infer(store, snapshot(clock, [7]))
    assert store.latest().selected_track_id is None
    assert store.latest().pending_click.x == 500 / 640
