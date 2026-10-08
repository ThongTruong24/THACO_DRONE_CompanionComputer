import numpy as np
import pytest
from vision_v1.pipeline import _resize_frame
from vision_v1.video_geometry import fit_output_size


@pytest.mark.parametrize("source,expected", [
    ((1280, 720), (640, 360)), ((1920, 1080), (640, 360)),
    ((640, 480), (640, 480)), ((320, 240), (320, 240)),
    ((720, 1280), (252, 448)),
])
def test_fit_preserves_exact_ratio_and_never_upscales(source, expected):
    result = fit_output_size(*source, 640, 480)
    assert result == expected
    assert result[0] * source[1] == result[1] * source[0]
    assert result[0] <= min(source[0], 640)
    assert result[1] <= min(source[1], 480)


@pytest.mark.parametrize("values", [(0, 720, 640, 480), (1280, 720, -1, 480), (2, 2, 640, 480)])
def test_invalid_or_unrepresentable_geometry_is_rejected(values):
    with pytest.raises(ValueError):
        fit_output_size(*values)


def test_circle_remains_round_and_full_frame_corners_are_kept():
    import cv2

    frame = np.zeros((720, 1280, 3), dtype=np.uint8)
    cv2.circle(frame, (640, 360), 100, (255, 255, 255), -1)
    frame[:10, :10] = (0, 255, 0)
    frame[-10:, -10:] = (0, 0, 255)
    resized = _resize_frame(frame, *fit_output_size(1280, 720, 640, 480))
    ys, xs = np.where(np.all(resized > 200, axis=2))
    assert abs((xs.max() - xs.min()) - (ys.max() - ys.min())) <= 1
    assert tuple(resized[0, 0]) == (0, 255, 0)
    assert tuple(resized[-1, -1]) == (0, 0, 255)
