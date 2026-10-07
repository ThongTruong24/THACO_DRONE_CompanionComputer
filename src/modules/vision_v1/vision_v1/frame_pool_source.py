"""FramePool adapter with a bounded latest-frame mailbox."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from .latest_value import LatestValue


@dataclass(frozen=True)
class InferenceFrame:
    source_timestamp_us: int
    generation: int
    source_seq: int
    color: object
    depth: object
    depth_scale: float


class FramePoolInferenceSource:
    def __init__(self, pool_dir: str = "/run/frame_pool", reader=None):
        if reader is None:
            from frame_pool import FramePoolReader

            reader = FramePoolReader(pool_dir)
        self.reader = reader
        self.mailbox: LatestValue[InferenceFrame] = LatestValue()
        self.received_frames = 0
        self.torn_frames = 0

    def submit_descriptor(self, generation: int, slot: int, seq: int) -> bool:
        frame = self.reader.read_frame(generation, slot, seq)
        if frame is None:
            self.torn_frames += 1
            return False

        timestamp_us, color, depth = frame
        header = getattr(self.reader, "header", None) or {}
        packet = InferenceFrame(
            source_timestamp_us=int(timestamp_us),
            generation=int(generation),
            source_seq=int(seq),
            color=color,
            depth=depth,
            depth_scale=float(header.get("depth_scale", 0.001)),
        )
        if not self.mailbox.put(packet):
            return False
        self.received_frames += 1
        return True

    def wait_next(self, last_version: int, timeout: Optional[float] = None):
        return self.mailbox.wait_next(last_version, timeout)

    def signal_stop(self) -> None:
        self.mailbox.close()

    def close_reader(self) -> None:
        self.reader.close()

    @property
    def dropped_frames(self) -> int:
        return self.mailbox.dropped
