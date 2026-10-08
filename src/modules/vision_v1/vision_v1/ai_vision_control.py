"""Thread-safe AI control flags and persistent target selection by track ID."""
from dataclasses import dataclass, replace
import math
import threading
import time

from .detection_types import DetectionSnapshot, clipped_bbox


@dataclass(frozen=True)
class PendingClick:
    x: float
    y: float
    deadline_ns: int
    control_revision: int
    event_id: int


@dataclass(frozen=True)
class AiVisionControlState:
    bounding_box: bool = False
    tracking: bool = False
    following: bool = False
    selected_track_id: int | None = None
    pending_click: PendingClick | None = None


class AiVisionControlStore:
    def __init__(self, clock_ns=None) -> None:
        self._lock = threading.Lock()
        self._state = AiVisionControlState()
        self._control_revision = 0
        self._click_revision = 0
        self._clock_ns = clock_ns or time.monotonic_ns

    def update(self, bounding_box: bool, tracking: bool, following: bool) -> None:
        with self._lock:
            bounding_box = bool(bounding_box)
            tracking = bounding_box and bool(tracking)
            if (bounding_box, tracking) != (self._state.bounding_box, self._state.tracking):
                self._control_revision += 1
            self._state = AiVisionControlState(
                bounding_box=bounding_box,
                tracking=tracking,
                following=bool(following),
                selected_track_id=self._state.selected_track_id if tracking else None,
                pending_click=self._state.pending_click if tracking else None,
            )

    def select_track_point(
        self,
        x: float,
        y: float,
        radius: float,
        snapshot: DetectionSnapshot | None,
        *,
        max_age_seconds: float,
        now_ns: int | None = None,
    ) -> bool:
        """Toggle a containing tracked bbox, or retain the newest click for one second."""
        if not all(math.isfinite(value) and 0.0 <= value <= 1.0 for value in (x, y, radius)):
            return False
        now = self._clock_ns() if now_ns is None else int(now_ns)
        with self._lock:
            if not (self._state.bounding_box and self._state.tracking):
                return False
            self._click_revision += 1
            pending = PendingClick(x, y, now + 1_000_000_000, self._control_revision, self._click_revision)
            self._state = replace(self._state, pending_click=pending)
        return self._try_pending(pending, snapshot, max_age_seconds, now, now_ns is None)

    def on_detection_snapshot(
        self, snapshot: DetectionSnapshot | None, *, max_age_seconds: float,
        now_ns: int | None = None,
    ) -> bool:
        """Reevaluate a pending event once new inference metadata is available."""
        now = self._clock_ns() if now_ns is None else int(now_ns)
        with self._lock:
            self._expire_pending(now)
            pending = self._state.pending_click
        if pending is None:
            return False
        return self._try_pending(pending, snapshot, max_age_seconds, now, now_ns is None)

    def _try_pending(
        self, pending: PendingClick, snapshot: DetectionSnapshot | None,
        max_age_seconds: float, now: int, live_clock: bool,
    ) -> bool:
        if snapshot is None or snapshot.age_seconds(now) > max_age_seconds:
            return False
        if snapshot.source_width <= 0 or snapshot.source_height <= 0:
            return False

        px = pending.x * snapshot.source_width
        py = pending.y * snapshot.source_height
        candidate = None
        for detection in snapshot.detections:
            if detection.track_id is None or detection.track_id < 0:
                continue
            bounds = clipped_bbox(detection, snapshot.source_width, snapshot.source_height)
            if bounds is None:
                continue
            x1, y1, x2, y2 = bounds
            if not (x1 <= px <= x2 and y1 <= py <= y2):
                continue
            distance = math.hypot(
                (x1 + x2) * 0.5 / snapshot.source_width - pending.x,
                (y1 + y2) * 0.5 / snapshot.source_height - pending.y,
            )
            key = (distance, detection.track_id)
            if candidate is None or key < candidate:
                candidate = key

        with self._lock:
            # Geometry runs outside the lock. OFF/ON, expiry or a newer click invalidates it.
            current_ns = self._clock_ns() if live_clock else now
            self._expire_pending(current_ns)
            if (pending != self._state.pending_click
                    or pending.control_revision != self._control_revision
                    or not self._state.tracking or candidate is None):
                return False
            selected = None if candidate[1] == self._state.selected_track_id else candidate[1]
            self._state = replace(self._state, selected_track_id=selected, pending_click=None)
        return True

    def _expire_pending(self, now_ns: int) -> None:
        pending = self._state.pending_click
        if pending is not None and now_ns >= pending.deadline_ns:
            self._state = replace(self._state, pending_click=None)

    def latest(self) -> AiVisionControlState:
        with self._lock:
            self._expire_pending(self._clock_ns())
            return self._state
