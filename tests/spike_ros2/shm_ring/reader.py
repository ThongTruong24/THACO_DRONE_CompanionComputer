#!/usr/bin/env python3
"""Spike 2 (T3): reader shm ring theo spec §5.10.

Với mỗi descriptor mới nhất: copy slot ngay, đọc lại seq, kiểm nội dung, rồi ngủ ngẫu nhiên
(giả lập YOLO). --naive dùng thẳng vùng nhớ sau khi ngủ (không copy), làm đối chứng.
--no-seqcheck bỏ kiểm seq, để chứng minh test bắt được frame xé.
"""
import argparse
import mmap
import random
import select
import socket
import struct
import sys
import time

import numpy as np

W, H = 640, 480
COLOR, DEPTH = W * H * 3, W * H * 2
SLOT_HDR = 64
SLOT_SIZE = (SLOT_HDR + COLOR + DEPTH + 63) // 64 * 64
FILE_HDR = 64
END = 0xFFFFFFFF


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=47001)
    ap.add_argument("--path", default="/dev/shm/spike_pool")
    ap.add_argument("--max-sleep", type=float, default=0.4)
    ap.add_argument("--naive", action="store_true")
    ap.add_argument("--no-seqcheck", action="store_true")
    ap.add_argument("--expect-torn", choices=["zero", "nonzero", "any"], default="zero")
    ap.add_argument("--max-copy-ms", type=float, default=0.0)
    ap.add_argument("--label", default="")
    a = ap.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("127.0.0.1", a.port))
    print("reader: ready", flush=True)

    # Chờ file pool do writer tạo.
    t0 = time.time()
    mm = None
    first = None
    sock.settimeout(15)
    try:
        first = sock.recvfrom(16)[0]
    except socket.timeout:
        print("FAIL: không nhận được descriptor nào")
        return 1
    while mm is None and time.time() - t0 < 5:
        try:
            with open(a.path, "rb") as f:
                mm = mmap.mmap(f.fileno(), 0, prot=mmap.PROT_READ)
        except (FileNotFoundError, ValueError):
            time.sleep(0.01)
    if mm is None:
        print("FAIL: không mở được pool")
        return 1

    rnd = random.Random(1234)
    pending = first
    written = 0
    processed = dropped_seq = torn = superseded = 0
    copy_ns = []
    cpu0 = time.process_time()
    ended = False
    sock.setblocking(True)

    while not ended:
        # Gom descriptor, chỉ giữ cái mới nhất (QoS depth 1).
        latest = None
        pkts = [pending] if pending else []
        pending = None
        if not pkts:
            r, _, _ = select.select([sock], [], [], 5)
            if not r:
                break
            pkts.append(sock.recvfrom(16)[0])
        while select.select([sock], [], [], 0)[0]:
            pkts.append(sock.recvfrom(16)[0])
        for p in pkts:
            slot, _, seq = struct.unpack("<IIQ", p)
            if slot == END:
                written = seq
                ended = True
            else:
                if latest is not None:
                    superseded += 1
                latest = (slot, seq)
        if latest is None:
            continue
        slot, seq = latest
        off = FILE_HDR + SLOT_SIZE * slot
        expect = (seq // 2) & 0xFF

        if a.naive:
            time.sleep(rnd.uniform(0, a.max_sleep))  # dùng vùng nhớ trực tiếp sau khi "suy luận"
            color = np.frombuffer(mm, dtype=np.uint8, count=COLOR, offset=off + SLOT_HDR)
            depth = np.frombuffer(mm, dtype=np.uint8, count=DEPTH, offset=off + SLOT_HDR + COLOR)
        else:
            if not a.no_seqcheck and struct.unpack_from("<Q", mm, off)[0] != seq:
                dropped_seq += 1
                time.sleep(rnd.uniform(0, a.max_sleep))
                continue
            t = time.perf_counter_ns()
            color = np.frombuffer(mm, dtype=np.uint8, count=COLOR, offset=off + SLOT_HDR).copy()
            depth = np.frombuffer(mm, dtype=np.uint8, count=DEPTH, offset=off + SLOT_HDR + COLOR).copy()
            copy_ns.append(time.perf_counter_ns() - t)
            if not a.no_seqcheck and struct.unpack_from("<Q", mm, off)[0] != seq:
                dropped_seq += 1
                time.sleep(rnd.uniform(0, a.max_sleep))
                continue

        ok = (color.min() == expect == color.max()) and (depth.min() == expect == depth.max())
        processed += 1
        if not ok:
            torn += 1
        if not a.naive:
            time.sleep(rnd.uniform(0, a.max_sleep))  # "YOLO" chạy trên bản copy riêng

    cpu = time.process_time() - cpu0
    avg_copy = (sum(copy_ns) / len(copy_ns) / 1e6) if copy_ns else 0.0
    max_copy = (max(copy_ns) / 1e6) if copy_ns else 0.0
    drop_rate = 100.0 * (1 - processed / written) if written else 0.0
    print(f"reader[{a.label}]: written={written} processed={processed} "
          f"superseded={superseded} dropped_seq={dropped_seq} torn_escaped={torn}")
    print(f"reader[{a.label}]: copy avg={avg_copy:.3f} ms max={max_copy:.3f} ms  "
          f"drop_rate={drop_rate:.1f}%  reader_cpu={cpu:.2f}s")

    rc = 0
    if a.expect_torn == "zero" and torn != 0:
        print("FAIL: có frame xé lọt qua"); rc = 1
    if a.expect_torn == "nonzero" and torn == 0:
        print("FAIL: đối chứng không thấy frame xé, test không đủ nhạy"); rc = 1
    if a.max_copy_ms and avg_copy > a.max_copy_ms:
        print(f"FAIL: copy trung bình {avg_copy:.3f} ms > {a.max_copy_ms} ms"); rc = 1
    print(f"[{'PASS' if rc == 0 else 'FAIL'}] {a.label}")
    return rc


if __name__ == "__main__":
    sys.exit(main())
