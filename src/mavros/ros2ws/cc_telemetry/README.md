# CC telemetry (C++ / ROS 2 Jazzy)

Companion identity: system **1**, component **191**. Transport is exclusively
MAVROS UDP 14542 → router 127.0.0.1:14541 → learned QGC peer UDP 14550.
No UART ownership, flight commands, camera device acquisition or Docker socket.

## Modules

- `collectors.cpp`: read-only links/camera/network/vision snapshots.
- `parameter_store.cpp`: numeric registry, validation, desired/active separation,
  atomic persistence under `/var/lib/cc-telemetry`.
- `node.cpp`: ROS timers, typed parameter requests, paced responses, diagnostics.
- `cc_interfaces`: typed parameter/event bridge to `plugin_agridrone`.
- `config/cc_telemetry.yaml`: sources, rates, public RTSP URL, write policy.

## Start and observe

Built into the MAVROS image; entrypoint starts the C++ node automatically.
Inside the container, source both environments before ROS CLI commands:

```bash
source /opt/ros/jazzy/setup.bash
source /opt/cc_ws/setup.bash
ros2 node list
ros2 topic echo /cc_telemetry/diagnostics
ros2 topic echo /drone_mavros/cc_telemetry/send_links
```

This package does not replace the independent FC observer `telemetry_demo.py`.

## QGroundControl connection

1. Connect laptop to AP_DRONE.
2. In QGC Application Settings → Comm Links, add a UDP link.
3. Add target `192.168.10.1:14550` (use Pi's reachable LAN address instead on LAN).
4. Connect: QGC must send initial traffic so router learns the return endpoint.
5. Never add a broadcast endpoint on UDP 14550.

### Inspect CC telemetry

QGC must be compiled with the same **thaco_common** dialect. Copying an XML into
an installed QGC binary does not add message support.

Open Analyze Tools → MAVLink Inspector (menu placement varies by version), select
system 1/component 191, and inspect:

| Message | ID | Default rate |
|---|---:|---:|
| CC_TELEMETRY_LINKS | 42010 | 1 Hz |
| CC_TELEMETRY_CAMERA | 42011 | 0.2 Hz |
| CC_TELEMETRY_NETWORK | 42012 | 0.2 Hz |
| CC_TELEMETRY_VISION | 42013 | 1 Hz |

Inspector is read-only. A stock QGC build may not decode these messages at all.
Successful packet capture is not proof that a particular QGC UI supports them.
RTSP URL telemetry does not automatically open video: configure Video Settings
separately, using the reachable RTSP URL and a QGC build with suitable video support.

### Edit CC parameters

The server supports numeric PARAM_REQUEST_LIST / PARAM_REQUEST_READ / PARAM_SET
and numeric PARAM_EXT equivalents. Values are **MAV_PARAM_TYPE_REAL32 (9)**;
integer-like settings require integral values. Names fit the 16-byte MAVLink limit.

**All writes must target 1.191**, never the FC component 1.1. Read/list can target
component 0. Write broadcasts are ignored. The default is remote writes disabled.
Enable only on a trusted link by changing `allow_remote_writes` in the mounted
YAML and recreating only MAVROS. MAVLink sender IDs are not authentication.

| Parameter | Meaning | Apply |
|---|---|---|
| CC_LINK_HZ | links frequency, 0.1–10 Hz | live |
| CC_CAM_HZ | camera frequency, 0.1–10 Hz | live |
| CC_NET_HZ | network frequency, 0.1–10 Hz | live |
| CC_VIS_HZ | vision frequency, 0.1–10 Hz | live |
| CC_CAM_W / CC_CAM_H | desired camera dimensions | pending only |
| CC_CAM_FPS | desired camera FPS | pending only |
| CC_CAM_KBPS | desired encoder bitrate | pending only |
| CC_YOLO_CONF | desired threshold, 0–1 | pending only |

Camera/vision entries are registered only if their configuration loads at startup.
The pending values are persisted **only in CC state**: they do not modify producer
YAML, do not restart containers and do not change the active video pipeline.
Even after CC restarts, pending is not promoted to applied. An apply adapter is
not implemented yet. PARAM_VALUE confirms the server's desired value, not runtime
application. Diagnostics report desired and active baseline separately.

To test from a supported QGC parameter UI/custom CC panel: explicitly select the
CC component, refresh, change CC_LINK_HZ from 1 to 2, read it back, confirm packet
frequency changed, then restore 1. Check rejection and Messages with writes off.
A generic stock QGC Parameters page is **not guaranteed** to discover/edit an
arbitrary onboard computer component; source/version is needed for UI integration.
String PARAM_EXT (URL/model editing), component metadata and custom QGC UI are
not implemented in this first pass. Their presence must not be inferred from
support for numeric PARAM_EXT packets.

### View logs

Open QGC Vehicle Messages/Messages and look for `CC ...` STATUSTEXT from 1.191.
Short ASCII events are limited to 50 bytes and at most one per second. Logs may
be dropped/coalesced under load; MAVLink has no STATUSTEXT delivery ACK. QGC's
PX4 MAVLink Console is a flight-controller shell, not a CC log console.

Full local logs:

```bash
docker logs --since 5m drone-edge-drone-mavros-1
```

No boot-history replay is guaranteed. No Wi-Fi password is transmitted or logged.

## Data provenance and unavailable values

Do not confuse configured FPS with measured FPS. Camera/YOLO snapshots describe
configuration; runtime acquisition, effective overrides and auto serial discovery
are not verified. Network IPs are measured; AP fields come from configuration.

Current wire-compatible conventions (consumers must implement these explicitly):

- UART counters `4294967295`: unavailable, **not real byte counters**.
- FC bitrate and vision inference FPS `-1`: unavailable.
- Network DHCP/service/client count and detections count `255`: unavailable.
- Empty serial: unknown/automatic; empty WPA password: intentionally redacted.
- SIYI/running bits not asserted: status unknown, not proof of stopped service.

These conventions are not yet formally defined in the dialect XML; diagnostics
remain authoritative for availability. Real producer metrics and validity fields
need a subsequent compatible contract update. CPU/RAM/temperature are not covered
by the four current custom messages.

## Verification status

Implementation is under build/test. Do not treat this README as a claim of live
Pi or QGC UI validation; see the execution walkthrough for actual test evidence.
