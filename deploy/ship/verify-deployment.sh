#!/usr/bin/env bash
# Read-only verification; no provisioning, restart or package install.
set -Eeuo pipefail
# shellcheck source=deploy/lib/common.sh
source "$(dirname "$0")/../lib/common.sh"
load_deploy_env
COMPOSE_FILE="${COMPOSE_FILE:-$DEPLOY_ROOT/docker-compose.yml}"
COMPOSE=(docker compose -f "$COMPOSE_FILE" --profile mavros --profile vision)
"${COMPOSE[@]}" config --quiet
SERVICES=("$@")
if ((${#SERVICES[@]} == 0)); then SERVICES=("${CORE_SERVICES[@]}"); fi
# Running containers can still be creating IPC sockets, AP interfaces or listeners.
wait_for() {
    local attempt
    for ((attempt=0; attempt<60; attempt++)); do
        if "$@"; then return 0; fi
        sleep 1
    done
    die "Runtime readiness timed out: $*"
}
for requested in "${SERVICES[@]}"; do
    service="$(service_name "$requested")"
    [[ "$service" != core && "$service" != drone-vision-base ]] || die 'Verification needs runtime service names'
    id="$("${COMPOSE[@]}" ps -q "$service")"
    [[ -n "$id" ]] || die "Service not running: $service"
    ready=false
    for ((attempt=0; attempt<60; attempt++)); do
        state="$(docker inspect --format '{{.State.Status}} {{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}' "$id")"
        if [[ "$state" == 'running healthy' || "$state" == 'running none' ]]; then ready=true; break; fi
        [[ "$state" != *unhealthy ]] || die "Unhealthy service: $service"
        sleep 1
    done
    [[ "$ready" == true ]] || die "Service did not become ready: $service"
    docker image inspect "$(docker inspect --format '{{.Image}}' "$id")" |
        python3 "$DEPLOY_ROOT/deploy/build/validate_image.py" config
    case "$service" in
        cc-agent)
            docker exec "$id" test -x /usr/local/bin/drone-companion-agent
            wait_for docker exec "$id" test -S /run/drone/hw_manager.sock
            wait_for docker exec "$id" test -S /run/drone/cc_agent.sock ;;
        mavlink-router-controller)
            config="${MAVLINK_CONFIG:-/etc/mavlink-router/main.conf}"
            wait_for docker exec "$id" test -S /run/drone/router.sock
            docker exec "$id" grep -Fq 'Address = 0.0.0.0' "$config"
            for port in 14550 14541 14600; do docker exec "$id" grep -Fq "Port = $port" "$config"; done
            ! docker exec "$id" grep -Eq 'Address = (192\.168\.10\.255|10\.[0-9]+\.[0-9]+\.255)|^\[UdpEndpoint LAN_(wlan|uap)' "$config" || die 'Broadcast telemetry loop route'
            if ! docker exec "$id" grep -Fq '[UartEndpoint FlightController]' "$config"; then echo 'INFO: Router UDP Standby (FC absent)'; fi
            wait_for bash -o pipefail -c "ss -uln | grep -Eq '(:|\.)14550\b'" ;;
        drone-networking)
            wait_for bash -o pipefail -c "ip -4 addr show uap0 | grep -q '192\.168\.10\.1/'" ;;
        camera-stream-controller)
            docker exec "$id" test -x /usr/local/bin/drone_camera_streamer
            wait_for bash -o pipefail -c "ss -tln | grep -Eq '(:|\.)8554\b'" ;;
        mavros)
            wait_for docker exec "$id" pgrep -f mavros_node >/dev/null
            wait_for docker exec "$id" pgrep -f cc_telemetry_node >/dev/null ;;
    esac
    printf 'PASS: %s running on ARM64\n' "$service"
done
