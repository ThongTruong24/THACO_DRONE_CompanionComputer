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


class Boxes(list):
    def __init__(self, track_ids=None):
        super().__init__([Box()])
        self.id = track_ids


class Result:
    def __init__(self, track_ids=None):
        self.boxes = Boxes(track_ids)
    names = {3: "vehicle"}


class FakeModel:
    names = {3: "vehicle"}

    def __init__(self, track_ids=None):
        self.track_ids = track_ids
        self.calls = []

    def track(self, frame, **kwargs):
        self.last_frame = frame
        self.last_kwargs = kwargs
        self.calls.append(kwargs)
        return [Result(self.track_ids)]


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
    assert detection.track_id is None
    assert model.last_kwargs["persist"] is True
    assert model.last_kwargs["tracker"] == "bytetrack.yaml"


def test_detector_keeps_track_id_as_int_across_inference_calls():
    model = FakeModel(track_ids=[Scalar(7.0)])
    detector = Detector("unused.pt", model=model)
    for sequence in (2, 4):
        result = detector.infer(FakeFrame(), source_timestamp_us=sequence, generation=1, source_seq=sequence)
        assert result.detections[0].track_id == 7
        assert isinstance(result.detections[0].track_id, int)
    assert all(call["persist"] is True for call in model.calls)
