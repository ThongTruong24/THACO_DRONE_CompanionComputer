#!/usr/bin/env bash
# deploy/deploy.sh - Shortcut tới deploy/ship/deploy.sh
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec bash "${SCRIPT_DIR}/ship/deploy.sh" "$@"
