"""YOLO inference that returns metadata and never renders video."""
from __future__ import annotations

import logging
import threading
from typing import Optional

from .detection_types import Detection, DetectionSnapshot
from .object_geometry import Intrinsics, sample_depth


LOG = logging.getLogger("vision_v1.detector")


class Detector:
    def __init__(self, model_path: str, imgsz: int = 320, confidence: float = 0.25, model=None):
        self.model_path = model_path
        self.imgsz = int(imgsz)
        self._confidence = float(confidence)
        self._lock = threading.Lock()
        self.model = model
        if self.model is None:
            self._load_model()

    def _load_model(self) -> None:
        try:
            from ultralytics import YOLO

            self.model = YOLO(self.model_path, task="detect")
            LOG.info("Loaded YOLO model from %s", self.model_path)
        except Exception as exc:
            LOG.warning("Could not load YOLO model (%s); empty detections will be returned", exc)
            self.model = None

    def set_confidence(self, confidence: float) -> None:
        with self._lock:
            self._confidence = float(confidence)

    @property
    def confidence(self) -> float:
        with self._lock:
            return self._confidence

    def infer(
        self,
        color_frame,
        *,
        source_timestamp_us: int,
        generation: int,
        source_seq: int,
        depth_frame=None,
        depth_scale: float = 0.001,
        intrinsics: Optional[Intrinsics] = None,
        min_distance_m: float = 0.3,
        max_distance_m: float = 10.0,
        sample_radius: int = 5,
    ) -> DetectionSnapshot:
        del intrinsics  # Reserved for future 3D position output; range currently uses aligned depth.
        height, width = color_frame.shape[:2]
        detections = []

        if self.model is not None:
            # Keep ID metadata continuous; control flags gate selection and rendering.
            results = self.model.track(
                color_frame,
                persist=True,
                tracker="bytetrack.yaml",
                imgsz=self.imgsz,
                conf=self.confidence,
                device="cpu",
                verbose=False,
            )
            result = results[0]
            boxes = result.boxes
            names = getattr(result, "names", None) or getattr(self.model, "names", {})

            if boxes is not None:
                track_ids = getattr(boxes, "id", None)
                depth_height = depth_frame.shape[0] if depth_frame is not None else 0
                depth_width = depth_frame.shape[1] if depth_frame is not None else 0
                scale_x = depth_width / width if width and depth_width else 1.0
                scale_y = depth_height / height if height and depth_height else 1.0

                for index, box in enumerate(boxes):
                    x1, y1, x2, y2 = [float(value) for value in box.xyxy[0].tolist()]
                    class_id = int(_scalar(box.cls[0]))
                    confidence = float(_scalar(box.conf[0]))
                    if isinstance(names, dict):
                        class_name = str(names.get(class_id, class_id))
                    else:
                        class_name = str(names[class_id]) if class_id < len(names) else str(class_id)

                    distance_m = None
                    if depth_frame is not None:
                        center_x = int(((x1 + x2) * 0.5) * scale_x)
                        center_y = int(((y1 + y2) * 0.5) * scale_y)
                        distance_m = sample_depth(
                            depth_frame,
                            center_x,
                            center_y,
                            sample_radius,
                            min_distance_m,
                            max_distance_m,
                            depth_scale,
                        )

                    detections.append(
                        Detection(
                            x1=x1,
                            y1=y1,
                            x2=x2,
                            y2=y2,
                            class_id=class_id,
                            class_name=class_name,
                            confidence=confidence,
                            distance_m=distance_m,
                            track_id=None if track_ids is None else int(_scalar(track_ids[index])),
                        )
                    )

        return DetectionSnapshot.create(
            source_timestamp_us=source_timestamp_us,
            source_width=width,
            source_height=height,
            generation=generation,
            source_seq=source_seq,
            detections=detections,
        )


def _scalar(value):
    return value.item() if hasattr(value, "item") else value
