from vision_v1.publisher import VisionPublisher, make_pipeline_desc


def test_publisher_pipeline_is_bounded_and_low_latency():
    pipeline = make_pipeline_desc(640, 480, 30, 1000, "rtmp://127.0.0.1:1935/yolo")
    assert "block=false" in pipeline
    assert "max-buffers=2" in pipeline
    assert "leaky=downstream" in pipeline
    assert "tune=zerolatency" in pipeline
    assert "rtmp://127.0.0.1:1935/yolo" in pipeline


def test_publisher_fps_reconfiguration_updates_timing():
    publisher = VisionPublisher(640, 480, 30, 1000, "rtmp://127.0.0.1:1935/yolo", 2.0)
    publisher.set_fps(20)
    assert publisher.fps == 20
