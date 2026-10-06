"""YOLO Detector and depth annotation helper."""
import logging
from typing import Optional, Tuple
import numpy as np

try:
    from vision.object_geometry import estimate_roi_depth, deproject_pixel, Intrinsics
except ImportError:
    from .object_geometry import estimate_roi_depth, deproject_pixel, Intrinsics

LOG = logging.getLogger("vision.detector")


def sample_depth(depth_frame_np: np.ndarray, cx: int, cy: int, radius: int,
                 min_m: float, max_m: float, depth_scale: float) -> Optional[float]:
    """Return median depth (m) in a (2*radius+1) square centered at (cx, cy)."""
    h, w = depth_frame_np.shape
    x0, x1 = max(0, cx - radius), min(w, cx + radius + 1)
    y0, y1 = max(0, cy - radius), min(h, cy + radius + 1)
    patch = depth_frame_np[y0:y1, x0:x1].astype(np.float32) * depth_scale
    valid = patch[(patch >= min_m) & (patch <= max_m)]
    if valid.size == 0:
        return None
    return float(np.median(valid))


class Detector:
    def __init__(self, model_path: str, imgsz: int = 320, confidence: float = 0.25):
        self.model_path = model_path
        self.imgsz = imgsz
        self.confidence = confidence
        self.model = None
        self._load_model()

    def _load_model(self):
        try:
            from ultralytics import YOLO
            self.model = YOLO(self.model_path, task="detect")
            LOG.info(f"Loaded YOLO model from {self.model_path}")
        except Exception as e:
            LOG.warning(f"Could not load YOLO model ({e}). Mock detection will be used.")
            self.model = None

    def set_confidence(self, conf: float):
        self.confidence = conf

    def detect(
        self,
        color_frame: np.ndarray,
        depth_frame: Optional[np.ndarray] = None,
        depth_scale: float = 0.001,
        intrinsics: Optional[Intrinsics] = None,
        min_distance_m: float = 0.3,
        max_distance_m: float = 10.0,
        sample_radius: int = 5
    ) -> Tuple[np.ndarray, int]:
        """
        Run inference on color_frame, annotate bboxes, sample depth if available.
        Returns: (annotated_bgr_frame, detection_count)
        """
        if self.model is None:
            # Fallback when ultralytics is not available (e.g. mock / unit test)
            annotated = color_frame.copy()
            return annotated, 0

        import cv2

        results = self.model.predict(
            color_frame,
            imgsz=self.imgsz,
            conf=self.confidence,
            device="cpu",
            verbose=False
        )

        annotated = results[0].plot()
        boxes = results[0].boxes
        count = len(boxes) if boxes is not None else 0

        if depth_frame is not None and count > 0:
            scale_x = depth_frame.shape[1] / color_frame.shape[1]
            scale_y = depth_frame.shape[0] / color_frame.shape[0]

            for box in boxes:
                x1, y1, x2, y2 = box.xyxy[0].tolist()
                cx = int((x1 + x2) / 2)
                cy = int((y1 + y2) / 2)
                dcx = int(cx * scale_x)
                dcy = int(cy * scale_y)

                dist = sample_depth(
                    depth_frame, dcx, dcy, sample_radius,
                    min_distance_m, max_distance_m, depth_scale
                )
                if dist is not None:
                    label = f"{dist:.2f}m"
                    cv2.putText(
                        annotated,
                        label,
                        (cx - 20, max(int(y1) - 8, 12)),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.65,
                        (0, 255, 255),
                        2,
                        cv2.LINE_AA
                    )

        return annotated, count
