"""Thread-safe AI control flags and persistent target selection by track ID."""
from dataclasses import dataclass, replace
import math
import threading
from typing import Optional

from .detection_types import DetectionSnapshot


@dataclass(frozen=True)
class AiVisionControlState:
    bounding_box: bool = False
    tracking: bool = False
    following: bool = False
    selected_track_id: Optional[int] = None


class AiVisionControlStore:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._state = AiVisionControlState()
        self._control_revision = 0

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
            )

    def select_track_point(
        self,
        x: float,
        y: float,
        radius: float,
        snapshot: Optional[DetectionSnapshot],
        *,
        max_age_seconds: float,
        now_ns: Optional[int] = None,
    ) -> bool:
        """Select the nearest valid tracked bbox; an unsuccessful click keeps the target."""
        with self._lock:
            if not (self._state.bounding_box and self._state.tracking):
                return False
            revision = self._control_revision

        if not all(math.isfinite(value) and 0.0 <= value <= 1.0 for value in (x, y, radius)):
            return False
        if snapshot is None or snapshot.age_seconds(now_ns) > max_age_seconds:
            return False
        if snapshot.source_width <= 0 or snapshot.source_height <= 0:
            return False

        px = x * snapshot.source_width
        py = y * snapshot.source_height
        radius_px = max(1.0, radius * snapshot.source_width)
        candidate = None
        for detection in snapshot.detections:
            if detection.track_id is None:
                continue
            if not (detection.x1 <= px <= detection.x2 and detection.y1 <= py <= detection.y2):
                continue
            distance = math.hypot(
                (detection.x1 + detection.x2) * 0.5 - px,
                (detection.y1 + detection.y2) * 0.5 - py,
            )
            if distance <= radius_px:
                key = (distance, detection.track_id)
                if candidate is None or key < candidate:
                    candidate = key

        if candidate is None:
            return False
        with self._lock:
            # A control change during selection must not resurrect a cleared target.
            if revision != self._control_revision or not self._state.tracking:
                return False
            self._state = replace(self._state, selected_track_id=candidate[1])
        return True

    def latest(self) -> AiVisionControlState:
        with self._lock:
            return self._state
