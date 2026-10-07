from vision_v1.detector import Detector


class Scalar:
    def __init__(self, value):
        self.value = value

    def item(self):
        return self.value


class Coordinates:
    def tolist(self):
        return [10, 20, 110, 220]


class Box:
    xyxy = [Coordinates()]
    cls = [Scalar(3)]
    conf = [Scalar(0.87)]


class Result:
    boxes = [Box()]
    names = {3: "vehicle"}


class FakeModel:
    names = {3: "vehicle"}

    def predict(self, frame, **kwargs):
        self.last_frame = frame
        self.last_kwargs = kwargs
        return [Result()]


class FakeFrame:
    shape = (480, 640, 3)


def test_detector_returns_metadata_not_annotated_frame():
    model = FakeModel()
    detector = Detector("unused.pt", imgsz=320, confidence=0.35, model=model)
    result = detector.infer(
        FakeFrame(),
        source_timestamp_us=123,
        generation=9,
        source_seq=44,
    )

    assert result.source_timestamp_us == 123
    assert result.source_width == 640
    assert result.source_height == 480
    assert result.generation == 9
    assert result.source_seq == 44
    assert len(result.detections) == 1
    detection = result.detections[0]
    assert detection.class_id == 3
    assert detection.class_name == "vehicle"
    assert detection.confidence == 0.87
    assert (detection.x1, detection.y1, detection.x2, detection.y2) == (10, 20, 110, 220)
