#!/usr/bin/env bash
# Explicit legacy/offline transport. Standard deployment uses Git + GHCR.
set -Eeuo pipefail
# shellcheck source=deploy/lib/common.sh
source "$(dirname "$0")/../lib/common.sh"
if [[ "${1:-}" == --help ]]; then
    echo 'Usage: offline-deploy.sh user@host [core|service]; build images locally first; provision private Pi config separately'
    exit 0
fi
registry_settings
REMOTE="${1:?Provide user@host}"; TARGET="$(service_name "${2:-core}")"
[[ "$REMOTE" =~ ^[A-Za-z0-9_.-]+@[A-Za-z0-9_.:-]+$ ]] || die 'Invalid SSH target'
[[ "$TARGET" != drone-vision-base ]] || die 'Vision base is not a runtime service'
SERVICES=("$TARGET")
if [[ "$TARGET" == core ]]; then SERVICES=("${CORE_SERVICES[@]}"); fi
# Legacy path is deliberately fixed, independent of the standard update script.
# shellcheck disable=SC2088 # Expanded on the remote host, not on WSL.
REMOTE_DIR='~/drone-edge'
for service in "${SERVICES[@]}"; do
    ref="$REGISTRY/$IMAGE_NAMESPACE/$(service_image "$service"):$IMAGE_TAG"
    docker image inspect "$ref" | python3 "$DEPLOY_ROOT/deploy/build/validate_image.py" config
    docker save "$ref" | gzip -1 | ssh "$REMOTE" 'gunzip -c | docker load'
done
ssh "$REMOTE" 'mkdir -p "$HOME/drone-edge"'
# Only Git-tracked application files: no private .env, wifi.json or runtime state.
rsync -avz --files-from=<(git -C "$DEPLOY_ROOT" ls-files -- src deploy docker-compose.yml Makefile ':(exclude)src/**/build/**') \
    "$DEPLOY_ROOT/" "$REMOTE:$REMOTE_DIR/"
printf -v remote_command 'bash -s -- %q %q %q %q' "$REGISTRY" "$IMAGE_NAMESPACE" "$IMAGE_TAG" "$TARGET"
# shellcheck disable=SC2029 # Arguments were individually quoted with printf %q.
ssh "$REMOTE" "$remote_command" <<'REMOTE_SCRIPT'
set -Eeuo pipefail
cd "$HOME/drone-edge"
export REGISTRY="$1" IMAGE_NAMESPACE="$2" IMAGE_TAG="$3"
compose=(docker compose)
case "$4" in
    core) services=(mavlink-router-controller cc-agent drone-networking camera-stream-controller) ;;
    mavros) compose+=(--profile mavros); services=(mavros) ;;
    drone-vision) compose+=(--profile vision); services=(drone-vision) ;;
    *) services=("$4") ;;
esac
"${compose[@]}" config --quiet
"${compose[@]}" up -d --no-build --pull never "${services[@]}"
"${compose[@]}" ps
bash deploy/ship/verify-deployment.sh "${services[@]}"
REMOTE_SCRIPT
