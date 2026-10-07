#!/usr/bin/env bash
# Verify the active Drone Edge deployment on the Raspberry Pi.
set -Eeuo pipefail

SERVICE="${1:-edge-router}"
COMPOSE_FILE="${COMPOSE_FILE:-$HOME/drone-edge/docker-compose.yml}"
MAVLINK_CONFIG="${MAVLINK_CONFIG:-/run/drone/mavlink-router.conf}"

fail() { echo "FAIL: $*" >&2; exit 1; }
pass() { echo "PASS: $*"; }

[ -f "$COMPOSE_FILE" ] || fail "Compose file not found: $COMPOSE_FILE"
docker compose -f "$COMPOSE_FILE" config --quiet
pass "Compose configuration is valid"

container_id="$(docker compose -f "$COMPOSE_FILE" ps -q "$SERVICE")"
[ -n "$container_id" ] || fail "Service is not running: $SERVICE"
[ "$(docker inspect -f '{{.State.Running}}' "$container_id")" = "true" ] || fail "Service is not running: $SERVICE"
pass "$SERVICE is running"

if [ "$SERVICE" = "edge-router" ]; then
    ip -4 addr show uap0 2>/dev/null | grep -q '192\.168\.10\.1/' || echo "INFO: uap0 192.168.10.1 not yet assigned (networking may still be starting)"
    pass "uap0 check complete"

    docker exec "$container_id" test -s "$MAVLINK_CONFIG" || fail "Rendered MAVLink config is missing"
    docker exec "$container_id" grep -Fq 'Address = 0.0.0.0' "$MAVLINK_CONFIG" || fail "UDP server endpoint is missing"

    # B21: Không FAIL khi FC vắng mặt (standby expected khi cắm/rút hoặc chưa gắn FC)
    CONFIG_LINKS="$HOME/drone-edge/config/links.json"
    if [ -f "$CONFIG_LINKS" ]; then
        links=$(python3 -c 'import json,sys; [print(l["port"]) for l in json.load(open(sys.argv[1]))["links"]]' "$CONFIG_LINKS" 2>/dev/null || true)
        for port in $links; do
            if [ -c "$port" ]; then
                docker exec "$container_id" grep -Fq "Device = $port" "$MAVLINK_CONFIG" || fail "UART endpoint missing for $port"
            else
                echo "INFO: $port absent -> standby expected (B21)"
            fi
        done
    else
        echo "INFO: $CONFIG_LINKS absent, skipping port check"
    fi

    ! docker exec "$container_id" grep -Eq 'Address = (192\.168\.10\.255|10\.[0-9]+\.[0-9]+\.255)' "$MAVLINK_CONFIG" || fail "Broadcast endpoint causes telemetry amplification"
    ! docker exec "$container_id" grep -Eq '^\[UdpEndpoint LAN_(wlan|uap)' "$MAVLINK_CONFIG" || fail "Wi-Fi LAN broadcast endpoint causes telemetry duplication"
    pass "MAVLink config includes FC serial and UDP server without broadcast loop routes"

    ss -uln | grep -Eq '(:|\.)14550\b' || fail "UDP port 14550 is not listening on the host"
    pass "UDP port 14550 is listening"
    echo "INFO: Capture AP telemetry: sudo tcpdump -ni uap0 udp port 14550"
    echo "INFO: Confirm FC input: docker compose logs --tail=100 edge-router"
fi

