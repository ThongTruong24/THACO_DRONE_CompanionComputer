"""Rendering of latest detection metadata onto the independent video path."""
from __future__ import annotations

import time
from typing import Optional, Tuple

from .detection_types import DetectionSnapshot, clipped_bbox
from .ai_vision_control import AiVisionControlState


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
        control: AiVisionControlState = AiVisionControlState(),
    ) -> Tuple[object, bool]:
        output = frame.copy() if copy_frame else frame
        if not control.bounding_box:
            return output, False
        current_ns = time.monotonic_ns() if now_ns is None else int(now_ns)
        if snapshot is None or snapshot.age_seconds(current_ns) > self.detection_max_age:
            return output, False
        if snapshot.source_width <= 0 or snapshot.source_height <= 0:
            return output, False

        import cv2

        height, width = output.shape[:2]
        scale_x = width / snapshot.source_width
        scale_y = height / snapshot.source_height
        # Draw selected boxes last so overlapping normal boxes cannot paint them green.
        detections = sorted(snapshot.detections, key=lambda item: (
            control.tracking and control.selected_track_id is not None
            and item.track_id == control.selected_track_id
        ))
        for detection in detections:
            bounds = clipped_bbox(detection, snapshot.source_width, snapshot.source_height)
            if bounds is None:
                continue
            x1 = _clamp(round(bounds[0] * scale_x), 0, max(0, width - 1))
            y1 = _clamp(round(bounds[1] * scale_y), 0, max(0, height - 1))
            x2 = _clamp(round(bounds[2] * scale_x), 0, max(0, width - 1))
            y2 = _clamp(round(bounds[3] * scale_y), 0, max(0, height - 1))
            if x2 <= x1 or y2 <= y1:
                continue

            label = f"{detection.class_name} {detection.confidence:.2f}"
            selected = (
                control.tracking
                and control.selected_track_id is not None
                and detection.track_id == control.selected_track_id
            )
            color = (0, 255, 255) if selected else (0, 255, 0)
            thickness = 3 if selected else 2
            if selected:
                label = f"SELECTED #{detection.track_id} {label}"
            if detection.distance_m is not None:
                label += f" {detection.distance_m:.2f}m"
            cv2.rectangle(output, (x1, y1), (x2, y2), color, thickness)
            cv2.putText(
                output,
                label,
                (x1, max(14, y1 - 6)),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5,
                color,
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
