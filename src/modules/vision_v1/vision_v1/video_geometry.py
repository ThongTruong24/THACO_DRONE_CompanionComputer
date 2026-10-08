"""Output dimensions derived from the decoded source, without cropping or stretching."""
import math


def fit_output_size(source_width: int, source_height: int, max_width: int, max_height: int):
    """Fit an exact aspect ratio inside bounds; align BGR rows and I420 dimensions.

    Width is a multiple of four (packed BGR matches GStreamer row stride), height
    is even. Unrepresentable sizes are rejected instead of silently stretching.
    """
    if min(source_width, source_height, max_width, max_height) <= 0:
        raise ValueError("source dimensions and output bounds must be positive")
    divisor = math.gcd(source_width, source_height)
    ratio_width, ratio_height = source_width // divisor, source_height // divisor
    step = math.lcm(4 // math.gcd(ratio_width, 4), 2 // math.gcd(ratio_height, 2))
    multiplier = min(divisor, max_width // ratio_width, max_height // ratio_height)
    multiplier = multiplier // step * step
    if multiplier == 0:
        raise ValueError("source aspect ratio cannot fit encoder alignment within output bounds")
    return ratio_width * multiplier, ratio_height * multiplier
