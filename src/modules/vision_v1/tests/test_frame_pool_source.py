from vision_v1.frame_pool_source import FramePoolInferenceSource


class FakeArray:
    shape = (480, 640, 3)


class FakeReader:
    header = {"depth_scale": 0.002}

    def __init__(self):
        self.closed = False

    def read_frame(self, generation, slot, seq):
        if seq == 3:
            return None
        return 1000, FakeArray(), None

    def close(self):
        self.closed = True


def test_frame_pool_source_keeps_latest_and_counts_torn_frames():
    source = FramePoolInferenceSource(reader=FakeReader())
    assert source.submit_descriptor(1, 0, 2)
    assert not source.submit_descriptor(1, 0, 3)
    version, frame = source.wait_next(0, timeout=0)
    assert version == 1
    assert frame.source_timestamp_us == 1000
    assert frame.depth_scale == 0.002
    assert source.torn_frames == 1
    source.signal_stop()
    source.close_reader()
    assert source.reader.closed
