"""Point selection over current detections; no persistent object identity."""
from dataclasses import dataclass
import math
import threading
import time

@dataclass(frozen=True)
class Detection:
    bbox: tuple[float, float, float, float]
    confidence: float
    label: str

def validate_point(point):
    if len(point) != 3 or any(not math.isfinite(v) or not 0 <= v <= 1 for v in point):
        raise ValueError('point x/y/radius must be finite normalized values in [0, 1]')

def choose_target(detections, point, frame_size):
    validate_point(point)
    width, height = frame_size
    if width <= 0 or height <= 0:
        raise ValueError('actual frame dimensions must be positive')
    px, py = point[0] * width, point[1] * height
    candidates = []
    for detection in detections:
        x1, y1, x2, y2 = detection.bbox
        if not all(math.isfinite(v) for v in (*detection.bbox, detection.confidence)) or x1 > x2 or y1 > y2:
            continue
        dx = max(x1 - px, 0, px - x2)
        dy = max(y1 - py, 0, py - y2)
        distance = dx * dx + dy * dy
        # MAV_CMD_CAMERA_TRACK_POINT radius is normalized by image WIDTH.
        # The dialect defines zero as one pixel, rather than an unbounded search.
        if distance > max(1, point[2] * width) ** 2:
            continue
        center = (px - (x1 + x2) / 2) ** 2 + (py - (y1 + y2) / 2) ** 2
        candidates.append(((distance, center, -detection.confidence), detection))
    return min(candidates, key=lambda item: item[0])[1] if candidates else None

class Selection:
    def __init__(self, ttl_seconds=5):
        self.ttl = ttl_seconds
        self.lock = threading.Lock()
        self.point = None
        self.generation = 0

    def set_point(self, x, y, radius, now=None, generation=None):
        validate_point((x, y, radius))
        with self.lock:
            self.point = ((x, y, radius), time.monotonic() if now is None else now)
            self.generation = self.generation + 1 if generation is None else generation

    def clear(self, generation=None):
        with self.lock:
            self.point = None
            self.generation = self.generation + 1 if generation is None else generation

    def choose(self, detections, frame_size, now=None):
        return self.evaluate(detections, frame_size, now)[0]

    def evaluate(self, detections, frame_size, now=None):
        now = time.monotonic() if now is None else now
        with self.lock:
            point = self.point
            selected = None
            if point is None or now - point[1] >= self.ttl:
                self.point = None
            else:
                selected = choose_target(detections, point[0], frame_size)
            width, height = frame_size
            bbox = None
            if selected is not None:
                x1, y1, x2, y2 = selected.bbox
                bbox = [max(0, min(1, value)) for value in (x1/width, y1/height, x2/width, y2/height)]
            return selected, {'generation': self.generation, 'bbox': bbox, 'width': width, 'height': height}
