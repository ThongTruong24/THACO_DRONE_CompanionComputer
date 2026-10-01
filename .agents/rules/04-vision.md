# Vision rules

- `drone-vision` is optional and runs only through Compose profile `vision`.
- The supported production pipeline is RTSP `/camera` to annotated `/yolo`.
- YOLO mode must not issue flight commands or MAVLink landing targets.
- Keep inference resource constraints and stale-frame behavior documented in
  `src/drone-vision/README.md`.
