"""Thread-safe latest-value store for immutable detection snapshots."""
from __future__ import annotations

import threading
import time
from typing import Optional

from .detection_types import DetectionSnapshot


class DetectionStore:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._snapshot: Optional[DetectionSnapshot] = None

    def update(self, snapshot: DetectionSnapshot) -> None:
        with self._lock:
            self._snapshot = snapshot

    def latest(
        self,
        max_age_seconds: Optional[float] = None,
        now_ns: Optional[int] = None,
    ) -> Optional[DetectionSnapshot]:
        with self._lock:
            snapshot = self._snapshot
        if snapshot is None:
            return None
        if max_age_seconds is None:
            return snapshot
        current_ns = time.monotonic_ns() if now_ns is None else int(now_ns)
        if snapshot.age_seconds(current_ns) > max_age_seconds:
            return None
        return snapshot

    def clear(self) -> None:
        with self._lock:
            self._snapshot = None
