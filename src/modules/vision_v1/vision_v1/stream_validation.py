"""Pure stream-route validation shared by startup and tests."""
from urllib.parse import urlparse


def validate_stream_routes(input_url: str, output_url: str):
    input_path = urlparse(input_url).path.rstrip("/")
    output_path = urlparse(output_url).path.rstrip("/")
    if input_path == "/yolo":
        return "input_url must not read the /yolo output path"
    if output_path == "/camera":
        return "output_url must not publish back to the /camera input path"
    if input_url == output_url or (input_path and input_path == output_path):
        return "input_url and output_url must use different stream paths"
    return None
