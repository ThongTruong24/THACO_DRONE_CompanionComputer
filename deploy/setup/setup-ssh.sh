#!/usr/bin/env bash
# Backward-compatible forwarder to setup.sh
exec "$(dirname "$0")/setup.sh" --ssh "$@"
