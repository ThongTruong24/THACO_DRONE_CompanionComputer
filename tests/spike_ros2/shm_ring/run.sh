#!/usr/bin/env bash
# Chạy một kịch bản shm ring: run.sh <label> <hz> <frames> <reader args...>
set -u
cd "$(dirname "$0")"
LABEL=$1; HZ=$2; FRAMES=$3; shift 3
PORT=$((47000 + RANDOM % 1000))
[ -x ./writer ] || g++ -O2 -std=c++17 -o writer writer.cpp || exit 1

python3 reader.py --label "$LABEL" --port "$PORT" "$@" &
RP=$!
sleep 0.5
./writer --hz "$HZ" --frames "$FRAMES" --port "$PORT"
wait "$RP"
