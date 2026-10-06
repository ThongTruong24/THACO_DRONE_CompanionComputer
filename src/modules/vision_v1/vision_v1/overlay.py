"""Rendering of latest detection metadata onto the independent video path."""
from __future__ import annotations

import time
from typing import Optional, Tuple

from .detection_types import DetectionSnapshot


class OverlayRenderer:
    def __init__(self, detection_max_age: float = 0.5):
        if detection_max_age <= 0:
            raise ValueError("detection_max_age must be positive")
        self.detection_max_age = float(detection_max_age)

    def render(
        self,
        frame,
        snapshot: Optional[DetectionSnapshot],
        *,
        now_ns: Optional[int] = None,
        copy_frame: bool = False,
    ) -> Tuple[object, bool]:
        output = frame.copy() if copy_frame else frame
        current_ns = time.monotonic_ns() if now_ns is None else int(now_ns)
        if snapshot is None or snapshot.age_seconds(current_ns) > self.detection_max_age:
            return output, False
        if snapshot.source_width <= 0 or snapshot.source_height <= 0:
            return output, False

        import cv2

        height, width = output.shape[:2]
        scale_x = width / snapshot.source_width
        scale_y = height / snapshot.source_height
        for detection in snapshot.detections:
            x1 = _clamp(round(detection.x1 * scale_x), 0, max(0, width - 1))
            y1 = _clamp(round(detection.y1 * scale_y), 0, max(0, height - 1))
            x2 = _clamp(round(detection.x2 * scale_x), 0, max(0, width - 1))
            y2 = _clamp(round(detection.y2 * scale_y), 0, max(0, height - 1))
            if x2 <= x1 or y2 <= y1:
                continue

            label = f"{detection.class_name} {detection.confidence:.2f}"
            if detection.distance_m is not None:
                label += f" {detection.distance_m:.2f}m"
            cv2.rectangle(output, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(
                output,
                label,
                (x1, max(14, y1 - 6)),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5,
                (0, 255, 0),
                1,
                cv2.LINE_AA,
            )
        return output, True


def scale_bbox(detection, source_width: int, source_height: int, target_width: int, target_height: int):
    if source_width <= 0 or source_height <= 0:
        raise ValueError("source dimensions must be positive")
    return (
        detection.x1 * target_width / source_width,
        detection.y1 * target_height / source_height,
        detection.x2 * target_width / source_width,
        detection.y2 * target_height / source_height,
    )


def _clamp(value: int, minimum: int, maximum: int) -> int:
    return min(maximum, max(minimum, int(value)))
