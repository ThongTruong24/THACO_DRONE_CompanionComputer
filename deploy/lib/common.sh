#!/usr/bin/env bash
# Shared naming and validation for build, publish, update and offline tools.

DEPLOY_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck disable=SC2034 # Used by scripts sourcing this library.
CORE_SERVICES=(mavlink-router-controller cc-agent drone-networking camera-stream-controller)

die() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

# Read simple dotenv assignments without executing shell code or replacing CLI env.
load_env_file() {
    local key value line
    local env_file="$1"
    [[ -f "$env_file" ]] || return 0
    while IFS= read -r line || [[ -n "$line" ]]; do
        line="${line%$'\r'}"
        [[ "$line" =~ ^[[:space:]]*(#|$) ]] && continue
        [[ "$line" == *=* ]] || die 'Invalid .env assignment'
        key="${line%%=*}"; value="${line#*=}"
        [[ "$key" =~ ^[A-Za-z_][A-Za-z0-9_]*$ ]] || die 'Invalid .env key'
        if [[ "$value" == \"*\" || "$value" == \'*\' ]]; then
            value="${value:1:${#value}-2}"
        fi
        if [[ ! -v "$key" ]]; then export "$key=$value"; fi
    done < "$env_file"
}

load_deploy_env() { load_env_file "$DEPLOY_ROOT/.env"; }

registry_settings() {
    load_deploy_env
    REGISTRY="${REGISTRY:-ghcr.io}"
    IMAGE_TAG="${IMAGE_TAG:-dev-ai-tracking-flow}"
    EXPECTED_BRANCH="${EXPECTED_BRANCH:-dev/ai_tracking_flow}"
    [[ "$REGISTRY" == ghcr.io ]] || die 'This workflow publishes to REGISTRY=ghcr.io'
    [[ "${IMAGE_NAMESPACE:-}" =~ ^[a-z0-9]+(-[a-z0-9]+)*$ ]] || die 'Set IMAGE_NAMESPACE to your lowercase GitHub owner/org (no token)'
    [[ "$IMAGE_TAG" =~ ^[A-Za-z0-9_][A-Za-z0-9_.-]{0,127}$ ]] || die 'Invalid IMAGE_TAG'
    [[ "$IMAGE_TAG" != latest ]] || die 'Use a commit SHA or development tag, not latest'
    export REGISTRY IMAGE_NAMESPACE IMAGE_TAG EXPECTED_BRANCH
}

service_name() {
    case "${1:-core}" in
        core|all) printf 'core\n' ;;
        mavlink|mavlink-router-controller|drone-mavlink) printf 'mavlink-router-controller\n' ;;
        cc|cc-agent|hw|hardware-manager) printf 'cc-agent\n' ;;
        networking|drone-networking|wifi-ap) printf 'drone-networking\n' ;;
        camera|camera-rtsp|camera-stream-controller|drone-camera-rtsp) printf 'camera-stream-controller\n' ;;
        mavros|drone-mavros) printf 'mavros\n' ;;
        vision|drone-vision) printf 'drone-vision\n' ;;
        vision-base|drone-vision-base) printf 'drone-vision-base\n' ;;
        *) die "Unknown service: $1 (Tailscale is MANUAL / UNRESOLVED)" ;;
    esac
}

service_image() {
    case "$1" in mavros) printf 'drone-mavros\n' ;; *) printf '%s\n' "$1" ;; esac
}

service_source() {
    case "$1" in drone-vision-base) printf '%s/src/drone-vision\n' "$DEPLOY_ROOT" ;; *) printf '%s/src/%s\n' "$DEPLOY_ROOT" "$1" ;; esac
}

require_clean_branch() {
    [[ -z "$(git -C "$DEPLOY_ROOT" status --porcelain)" ]] || die 'Working tree is dirty; review, commit and push before publishing/updating'
    [[ "$(git -C "$DEPLOY_ROOT" branch --show-current)" == "$EXPECTED_BRANCH" ]] || die "Expected branch $EXPECTED_BRANCH"
}

verify_registry_image() {
    local ref="$1" revision="${2:-}" manifest format
    manifest="$(docker buildx imagetools inspect --raw "$ref")"
    printf '%s' "$manifest" | python3 "$DEPLOY_ROOT/deploy/build/validate_image.py" manifest
    format='{{json .Image}}'
    if printf '%s' "$manifest" | python3 -c 'import json,sys; sys.exit("manifests" not in json.load(sys.stdin))'; then
        # Default .Image serialization can select the WSL host's amd64 platform.
        format='{{json (index .Image "linux/arm64")}}'
    fi
    docker buildx imagetools inspect --format "$format" "$ref" |
        python3 "$DEPLOY_ROOT/deploy/build/validate_image.py" config "$revision"
}
