#!/usr/bin/env bash
# Run on the Pi: Git source/config + registry images. No build or provisioning.
set -Eeuo pipefail
# shellcheck source=deploy/lib/common.sh
source "$(dirname "$0")/lib/common.sh"

PULL_GIT=true; CHECK_ONLY=false; TARGET=core; EXPECTED_REVISION=""
while (($#)); do
    case "$1" in
        --no-git-pull) PULL_GIT=false; shift ;;
        --check) CHECK_ONLY=true; shift ;;
        --service) TARGET="${2:?Missing service}"; shift 2 ;;
        --expected-revision) EXPECTED_REVISION="${2:?Missing revision}"; shift 2 ;;
        -h|--help) echo 'Usage: deploy/update.sh [--check] [--no-git-pull] [--service core|service] [--expected-revision SHA]'; exit 0 ;;
        *) die "Unknown argument: $1" ;;
    esac
done
registry_settings
TARGET="$(service_name "$TARGET")"
[[ "$TARGET" != drone-vision-base ]] || die 'Vision base is a build dependency, not a Compose service'
cd "$DEPLOY_ROOT"
[[ "$(uname -m)" == aarch64 || "$(uname -m)" == arm64 ]] || die 'Update must run on the ARM64 Raspberry Pi'
require_clean_branch
docker info >/dev/null
if [[ "$PULL_GIT" == true && "$CHECK_ONLY" == false ]]; then git pull --ff-only; fi
require_clean_branch
REVISION="$(git rev-parse HEAD)"
[[ -z "$EXPECTED_REVISION" || "$REVISION" == "$EXPECTED_REVISION" ]] || die 'Pi Git revision differs from the published revision'

COMPOSE=(docker compose -f "$DEPLOY_ROOT/docker-compose.yml")
if [[ "$TARGET" == core ]]; then
    SERVICES=("${CORE_SERVICES[@]}")
else
    SERVICES=("$TARGET")
    case "$TARGET" in
        mavros) COMPOSE+=(--profile mavros) ;;
        drone-vision) COMPOSE+=(--profile vision) ;;
    esac
fi
CONFIG_JSON="$("${COMPOSE[@]}" config --format json)"
mapfile -t SERVICES < <(printf '%s' "$CONFIG_JSON" | python3 -c '
import json,sys
d=json.load(sys.stdin)["services"]; selected=set()
def add(name):
    if name in selected: return
    selected.add(name)
    for dep in d[name].get("depends_on", {}): add(dep)
for name in sys.argv[1:]: add(name)
print("\n".join(sorted(selected)))
' "${SERVICES[@]}")
[[ ${#SERVICES[@]} -gt 0 ]] || die 'No services selected'
# Validate binds before Docker can accidentally create a directory for a file.
printf '%s' "$CONFIG_JSON" | python3 -c '
import json,pathlib,sys
d=json.load(sys.stdin)["services"]
for name in sys.argv[1:]:
    for mount in d[name].get("volumes", []):
        if mount["type"] != "bind": continue
        p=pathlib.Path(mount["source"])
        if str(p) == "/run/drone": continue
        if not p.exists(): raise SystemExit(f"ERROR: Missing runtime bind for {name}: {p}; see deploy/README.md")
        if mount["target"].endswith((".sh", ".py", ".yaml", ".yml", ".conf", ".pt", "/.env")) and not p.is_file():
            raise SystemExit(f"ERROR: Expected a file bind: {p}")
        if name == "drone-networking" and mount["target"] == "/app/config":
            wifi=p / "wifi.json"
            if not wifi.is_file(): raise SystemExit(f"ERROR: Provision private WiFi config first: {wifi}")
            json.loads(wifi.read_text())
' "${SERVICES[@]}"
if [[ "$CHECK_ONLY" == true ]]; then
    printf 'PASS: Pi checkout and Compose config; update services: %s\n' "${SERVICES[*]}"
    exit 0
fi
"${COMPOSE[@]}" pull "${SERVICES[@]}"
for service in "${SERVICES[@]}"; do
    ref="$(printf '%s' "$CONFIG_JSON" | python3 -c 'import json,sys; print(json.load(sys.stdin)["services"][sys.argv[1]]["image"])' "$service")"
    docker image inspect "$ref" | python3 "$DEPLOY_ROOT/deploy/build/validate_image.py" config "$REVISION"
done
# Prevent a second pull from moving a development tag after validation.
"${COMPOSE[@]}" up -d --no-build --pull never "${SERVICES[@]}"
"${COMPOSE[@]}" ps
COMPOSE_FILE="$DEPLOY_ROOT/docker-compose.yml" bash "$DEPLOY_ROOT/deploy/ship/verify-deployment.sh" "${SERVICES[@]}"
