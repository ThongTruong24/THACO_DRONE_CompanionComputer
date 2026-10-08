"""Immutable detection metadata shared between inference and video workers."""
from __future__ import annotations

from dataclasses import dataclass
import math
import time
from typing import Iterable, Optional, Tuple


@dataclass(frozen=True)
class Detection:
    x1: float
    y1: float
    x2: float
    y2: float
    class_id: int
    class_name: str
    confidence: float
    distance_m: Optional[float] = None
    track_id: Optional[int] = None


@dataclass(frozen=True)
class DetectionSnapshot:
    source_timestamp_us: int
    completed_monotonic_ns: int
    source_width: int
    source_height: int
    generation: int
    source_seq: int
    detections: Tuple[Detection, ...]

    @classmethod
    def create(
        cls,
        *,
        source_timestamp_us: int,
        source_width: int,
        source_height: int,
        generation: int,
        source_seq: int,
        detections: Iterable[Detection],
        completed_monotonic_ns: Optional[int] = None,
    ) -> "DetectionSnapshot":
        return cls(
            source_timestamp_us=int(source_timestamp_us),
            completed_monotonic_ns=(
                time.monotonic_ns()
                if completed_monotonic_ns is None
                else int(completed_monotonic_ns)
            ),
            source_width=int(source_width),
            source_height=int(source_height),
            generation=int(generation),
            source_seq=int(source_seq),
            detections=tuple(detections),
        )

    def age_seconds(self, now_ns: Optional[int] = None) -> float:
        current_ns = time.monotonic_ns() if now_ns is None else int(now_ns)
        return max(0.0, (current_ns - self.completed_monotonic_ns) / 1_000_000_000.0)


def clipped_bbox(detection: Detection, width: int, height: int):
    """The finite, visible bbox shared by hit testing and rendering."""
    coordinates = (detection.x1, detection.y1, detection.x2, detection.y2)
    if width <= 0 or height <= 0 or not all(math.isfinite(value) for value in coordinates):
        return None
    x1, y1 = max(0.0, detection.x1), max(0.0, detection.y1)
    x2, y2 = min(float(width), detection.x2), min(float(height), detection.y2)
    return None if x2 <= x1 or y2 <= y1 else (x1, y1, x2, y2)
