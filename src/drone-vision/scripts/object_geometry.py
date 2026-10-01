"""Pure geometry and depth-quality helpers for RGB-D object localization."""
from __future__ import annotations

from dataclasses import dataclass
from math import isfinite
from statistics import median
from typing import Iterable, Optional, Sequence


@dataclass(frozen=True)
class Intrinsics:
    fx: float
    fy: float
    cx: float
    cy: float


@dataclass(frozen=True)
class DepthEstimate:
    range_m: float
    valid_ratio: float
    mad_m: float
    sample_count: int


def estimate_roi_depth(
    depth_m: Sequence[Sequence[float]],
    bbox: Sequence[float],
    min_distance_m: float,
    max_distance_m: float,
    center_fraction: float = 0.5,
    minimum_samples: int = 12,
) -> Optional[DepthEstimate]:
    """Return robust median depth inside the centre of a 2D bounding box."""
    if not 0 < center_fraction <= 1:
        raise ValueError("center_fraction must be in (0, 1]")
    if min_distance_m <= 0 or max_distance_m <= min_distance_m:
        raise ValueError("invalid distance limits")
    if len(bbox) != 4 or not depth_m or not depth_m[0]:
        return None

    height, width = len(depth_m), len(depth_m[0])
    x1, y1, x2, y2 = bbox
    box_w, box_h = max(0.0, x2 - x1), max(0.0, y2 - y1)
    if box_w < 1 or box_h < 1:
        return None
    margin_x = box_w * (1 - center_fraction) / 2
    margin_y = box_h * (1 - center_fraction) / 2
    left = max(0, int(x1 + margin_x))
    right = min(width, int(x2 - margin_x + 0.999))
    top = max(0, int(y1 + margin_y))
    bottom = min(height, int(y2 - margin_y + 0.999))
    values = [
        value for row in depth_m[top:bottom] for value in row[left:right]
        if isfinite(value) and min_distance_m <= value <= max_distance_m
    ]
    total = max(1, (right - left) * (bottom - top))
    if len(values) < minimum_samples:
        return None
    mid = median(values)
    mad = median(abs(value - mid) for value in values)
    return DepthEstimate(mid, len(values) / total, mad, len(values))


def deproject_pixel(u: float, v: float, depth_m: float, intrinsics: Intrinsics) -> tuple[float, float, float]:
    """Project a rectified colour pixel plus metric depth into optical-frame XYZ."""
    if depth_m <= 0 or not isfinite(depth_m):
        raise ValueError("depth_m must be finite and positive")
    if intrinsics.fx <= 0 or intrinsics.fy <= 0:
        raise ValueError("focal lengths must be positive")
    return (
        (u - intrinsics.cx) * depth_m / intrinsics.fx,
        (v - intrinsics.cy) * depth_m / intrinsics.fy,
        depth_m,
    )
