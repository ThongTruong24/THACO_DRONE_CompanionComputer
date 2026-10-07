import math

import pytest

from vision_v1.ai_vision_control import AiVisionControlStore
from vision_v1.detection_types import Detection, DetectionSnapshot


NOW = 1_000_000_000


def bbox(x1, y1, x2, y2, track_id):
    return Detection(x1, y1, x2, y2, 1, "plant", 0.9, track_id=track_id)


def snapshot(detections, completed_ns=NOW):
    return DetectionSnapshot.create(
        source_timestamp_us=1, source_width=1000, source_height=600,
        generation=1, source_seq=2, detections=detections,
        completed_monotonic_ns=completed_ns,
    )


def select(store, detections, radius=0.02):
    return store.select_track_point(
        0.5, 0.5, radius, snapshot(detections), max_age_seconds=0.5, now_ns=NOW,
    )


@pytest.fixture
def store():
    control = AiVisionControlStore()
    control.update(True, True, False)
    assert select(control, [bbox(490, 290, 510, 310, 7)])
    return control


def test_point_inside_bbox_and_radius_selects_new_target(store):
    assert select(store, [bbox(490, 290, 520, 310, 8)])
    assert store.latest().selected_track_id == 8
    assert select(store, [bbox(480, 280, 520, 320, 9)])
    assert store.latest().selected_track_id == 9


@pytest.mark.parametrize("detections,radius", [
    ([bbox(501, 290, 503, 310, 8)], 0.02),  # close center, point outside bbox
    ([bbox(490, 290, 540, 310, 8)], 0.005),  # point inside, center outside radius
    ([bbox(499, 299, 503.02, 301, 8)], 0.0),  # center > 1 px away
    ([bbox(490, 290, 510, 310, None)], 0.02),
    ([], 0.02),
])
def test_invalid_candidates_keep_existing_target(store, detections, radius):
    assert not select(store, detections, radius)
    assert store.latest().selected_track_id == 7


def test_nearest_valid_center_wins_regardless_of_bbox_order(store):
    far = bbox(480, 290, 540, 310, 8)  # center 510
    near = bbox(490, 290, 514, 310, 9)  # center 502
    assert select(store, [far, near])
    assert store.latest().selected_track_id == 9


@pytest.mark.parametrize("ids", [(9, 8), (8, 9)])
def test_equal_distance_tie_break_uses_smaller_track_id(store, ids):
    assert select(store, [bbox(490, 290, 510, 310, track_id) for track_id in ids])
    assert store.latest().selected_track_id == 8


def test_closest_bbox_without_track_id_is_skipped(store):
    assert select(store, [bbox(490, 290, 510, 310, None), bbox(490, 290, 530, 310, 8)])
    assert store.latest().selected_track_id == 8


def test_zero_radius_accepts_center_within_one_pixel(store):
    assert select(store, [bbox(499, 299, 503, 301, 8)], radius=0.0)
    assert store.latest().selected_track_id == 8


@pytest.mark.parametrize("coordinates", [
    (500, 300, 520, 320), (480, 280, 500, 300),
    (500, 280, 520, 300), (480, 300, 500, 320),
])
def test_bbox_edges_are_inclusive_and_radius_scales_by_source_width(store, coordinates):
    assert select(store, [bbox(*coordinates, 8)], radius=0.02)
    assert store.latest().selected_track_id == 8


@pytest.mark.parametrize("value", [None, snapshot([]), snapshot([bbox(490, 290, 510, 310, 8)], NOW - 600_000_000)])
def test_missing_empty_or_stale_snapshot_keeps_target(store, value):
    assert not store.select_track_point(0.5, 0.5, 0.02, value, max_age_seconds=0.5, now_ns=NOW)
    assert store.latest().selected_track_id == 7


def test_selection_reuses_configured_max_age(store):
    value = snapshot([bbox(490, 290, 510, 310, 8)], NOW - 300_000_000)
    assert not store.select_track_point(0.5, 0.5, 0.02, value, max_age_seconds=0.2, now_ns=NOW)
    assert store.latest().selected_track_id == 7
    assert store.select_track_point(0.5, 0.5, 0.02, value, max_age_seconds=0.4, now_ns=NOW)
    assert store.latest().selected_track_id == 8


@pytest.mark.parametrize("flags", [(False, True, True), (True, False, True)])
def test_disabling_bbox_or_tracking_clears_target_and_ignores_clicks(store, flags):
    store.update(*flags)
    state = store.latest()
    assert state.selected_track_id is None
    assert not state.tracking
    assert state.following  # following is retained, without controlling selection
    assert not select(store, [bbox(490, 290, 510, 310, 8)])


def test_following_changes_only_stored_flag(store):
    store.update(True, True, True)
    assert store.latest().following
    assert store.latest().selected_track_id == 7
    store.update(True, True, False)
    assert not store.latest().following
    assert store.latest().selected_track_id == 7


@pytest.mark.parametrize("x,y,radius", [
    (math.nan, 0.5, 0.1), (0.5, math.inf, 0.1), (0.5, 0.5, math.nan),
    (-0.1, 0.5, 0.1), (0.5, 1.1, 0.1), (0.5, 0.5, -0.1), (0.5, 0.5, 1.1),
])
def test_invalid_normalized_point_keeps_target(store, x, y, radius):
    assert not store.select_track_point(x, y, radius, snapshot([bbox(490, 290, 510, 310, 8)]),
                                        max_age_seconds=0.5, now_ns=NOW)
    assert store.latest().selected_track_id == 7


def test_control_change_during_selection_cannot_restore_cleared_target(store, monkeypatch):
    original_hypot = math.hypot

    def change_control(*args):
        # Selection geometry runs outside the control lock.
        assert store.latest().selected_track_id == 7
        store.update(False, False, False)
        store.update(True, True, False)
        return original_hypot(*args)

    monkeypatch.setattr("vision_v1.ai_vision_control.math.hypot", change_control)
    assert not select(store, [bbox(490, 290, 510, 310, 8)])
    assert store.latest().selected_track_id is None
