import mmap
import os
import struct
import numpy as np

FILE_HDR_SIZE = 64
SLOT_HDR_SIZE = 64
MAGIC = b"CCFP"
VERSION = 1


class FramePoolReader:
    """
    Reader for shared memory frame pool according to spec 2026-10-04 section 5.10.
    """

    def __init__(self, pool_dir="/run/frame_pool"):
        self.pool_dir = pool_dir
        self.current_generation = None
        self.fd = None
        self.mm = None
        self.header = None

    def close(self):
        if self.mm is not None:
            try:
                self.mm.close()
            except Exception:
                pass
            self.mm = None
        if self.fd is not None:
            try:
                os.close(self.fd)
            except Exception:
                pass
            self.fd = None
        self.current_generation = None
        self.header = None

    def __del__(self):
        self.close()

    def _open_generation(self, generation):
        self.close()
        path = os.path.join(self.pool_dir, f"pool.{generation}")
        if not os.path.exists(path):
            return False

        try:
            self.fd = os.open(path, os.O_RDONLY)
            self.mm = mmap.mmap(self.fd, 0, prot=mmap.PROT_READ)
        except (OSError, ValueError):
            self.close()
            return False

        if len(self.mm) < FILE_HDR_SIZE:
            self.close()
            return False

        # struct FramePoolHeader:
        # char magic[4], uint32 version, uint32 generation, uint32 width, uint32 height,
        # char encoding_color[8], uint8 has_depth, uint8 reserved[3],
        # char encoding_depth[8], float depth_scale, uint32 slot_size, uint32 num_slots, uint8 pad[16]
        try:
            (magic, ver, gen, width, height, enc_color, has_depth,
             res0, res1, res2, enc_depth, depth_scale, slot_size,
             num_slots) = struct.unpack_from("<4sIIII8sBBBB8sfII", self.mm, 0)
        except struct.error:
            self.close()
            return False

        if magic != MAGIC or ver != VERSION or gen != generation:
            self.close()
            return False

        self.current_generation = generation
        self.header = {
            "version": ver,
            "generation": gen,
            "width": width,
            "height": height,
            "encoding_color": enc_color.rstrip(b"\x00").decode("ascii", errors="ignore"),
            "has_depth": bool(has_depth),
            "encoding_depth": enc_depth.rstrip(b"\x00").decode("ascii", errors="ignore"),
            "depth_scale": depth_scale,
            "slot_size": slot_size,
            "num_slots": num_slots,
        }
        return True

    def read_frame(self, generation, slot, seq):
        """
        Read a frame described by (generation, slot, seq).
        Returns (timestamp_us, color_array, depth_array_or_none) or None.
        """
        if self.current_generation != generation or self.mm is None:
            if not self._open_generation(generation):
                return None

        if slot >= self.header["num_slots"]:
            return None

        slot_offset = FILE_HDR_SIZE + slot * self.header["slot_size"]
        if slot_offset + SLOT_HDR_SIZE > len(self.mm):
            return None

        # Slot header: uint64 seq, uint64 timestamp, uint32 color_offset, uint32 color_size,
        # uint32 depth_offset, uint32 depth_size
        try:
            seq1, ts, color_off, color_size, depth_off, depth_size = struct.unpack_from(
                "<QQIIII", self.mm, slot_offset
            )
        except struct.error:
            return None

        # Check if overwritten or being written
        if seq1 != seq or (seq1 % 2 != 0):
            return None

        width = self.header["width"]
        height = self.header["height"]

        # Color buffer
        color_start = slot_offset + color_off
        if color_start + color_size > len(self.mm):
            return None

        color = np.frombuffer(
            self.mm, dtype=np.uint8, count=color_size, offset=color_start
        ).reshape((height, width, 3)).copy()

        depth = None
        if self.header["has_depth"] and depth_size > 0:
            depth_start = slot_offset + depth_off
            if depth_start + depth_size <= len(self.mm):
                depth = np.frombuffer(
                    self.mm, dtype=np.uint16, count=depth_size // 2, offset=depth_start
                ).reshape((height, width)).copy()

        # Re-check seq for tear detection
        try:
            seq2 = struct.unpack_from("<Q", self.mm, slot_offset)[0]
        except struct.error:
            return None

        if seq2 != seq1:
            return None

        return (ts, color, depth)
