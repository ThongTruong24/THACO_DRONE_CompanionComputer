#!/usr/bin/env bash
# Read-only runtime snapshot for CPU optimization validation on the Raspberry Pi.
set -euo pipefail

COMPOSE_FILE="${COMPOSE_FILE:-$HOME/drone-edge/docker-compose.yml}"
section() { printf '\n=== %s ===\n' "$1"; }

section "timestamp"
date --iso-8601=seconds

section "host"
uptime
printf 'CPU cores: '; nproc
free -h
if command -v vcgencmd >/dev/null 2>&1; then
  vcgencmd measure_temp || true
  vcgencmd get_throttled || true
fi
if [[ -r /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq ]]; then
  awk '{printf "cpu0 frequency: %.0f MHz\n", $1 / 1000}' /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
fi

section "containers"
docker stats --no-stream --format 'table {{.Name}}\t{{.CPUPerc}}\t{{.MemUsage}}\t{{.NetIO}}'

section "top processes"
ps -eo pid,ppid,comm,%cpu,%mem,psr,args --sort=-%cpu | head -n 25

section "top threads"
ps -eLo pid,tid,psr,pcpu,pmem,comm --sort=-pcpu | head -n 30

section "service state"
docker compose -f "$COMPOSE_FILE" --profile vision ps

section "camera and vision indicators"
docker compose -f "$COMPOSE_FILE" logs --tail=120 edge-camera 2>&1 |
  grep -E 'Pipeline|Encoder|FPS:|Dropped:|RealSense' | tail -n 30 || true
if docker compose -f "$COMPOSE_FILE" --profile vision ps --status running edge-vision 2>/dev/null | grep -q edge-vision; then
  docker compose -f "$COMPOSE_FILE" --profile vision logs --tail=120 edge-vision 2>&1 |
    grep -E 'Inference|Output|RTSP|YOLO' | tail -n 30 || true
else
  echo 'edge-vision is not running (expected when the optional profile is disabled).'
fi