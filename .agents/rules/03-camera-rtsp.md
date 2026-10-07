# Camera + RTSP rules

- `camera-stream-controller` owns RealSense/V4L2 capture, NEON processing, and MediaMTX `/camera`.
- FPV `/camera` must remain independent of Vision processing.
- Use `camera.yaml` and `mediamtx.yml` as the only stream configuration sources.
- See `src/camera-stream-controller/README.md` for URLs, operations and USB diagnostics.
