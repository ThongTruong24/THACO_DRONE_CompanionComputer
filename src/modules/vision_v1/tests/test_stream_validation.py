from vision_v1.stream_validation import validate_stream_routes


def test_valid_camera_to_yolo_route():
    assert validate_stream_routes(
        "rtsp://127.0.0.1:8554/camera",
        "rtmp://127.0.0.1:1935/yolo",
    ) is None


def test_rejects_feedback_routes():
    assert validate_stream_routes("rtsp://127.0.0.1:8554/yolo", "rtmp://127.0.0.1:1935/out")
    assert validate_stream_routes("rtsp://127.0.0.1:8554/in", "rtmp://127.0.0.1:1935/camera")
    assert validate_stream_routes("rtsp://host/camera", "rtmp://host/camera")
