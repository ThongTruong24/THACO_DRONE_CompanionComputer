#!/usr/bin/env bash
# Chạy toàn bộ spike T3 trên WSL. In bảng PASS/FAIL ở cuối.
set -u
cd "$(dirname "$0")"
FAILS=0
step() { echo; echo "=== $1"; }
run() { "$@" || FAILS=$((FAILS+1)); }

step "Build spike 1"
# shellcheck disable=SC1091
set +u; source /opt/ros/jazzy/setup.bash; set -u
# lo của WSL không có cờ MULTICAST nên discovery mặc định giữa process không chạy; dùng unicast trên lo.
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI="file://$PWD/ros2_api/cyclonedds_lo.xml"
unset ROS_AUTOMATIC_DISCOVERY_RANGE
cmake -S ros2_api -B ros2_api/build -DCMAKE_BUILD_TYPE=Release >/dev/null && \
  cmake --build ros2_api/build -j"$(nproc)" >/dev/null || { echo "build spike 1 lỗi"; exit 1; }
API=ros2_api/build/spike_ros2_api

step "Spike 1a: take()"
run $API take
step "Spike 1b: AsyncParametersClient, node vắng"
run $API param_absent
step "Spike 1c: AsyncParametersClient, node có mặt (process riêng)"
$API param_server & SP=$!
sleep 1
run $API param_present
kill $SP 2>/dev/null; wait $SP 2>/dev/null
step "Spike 1d: RSS idle node C++"
$API rss

step "Spike 2a: shm ring, kịch bản spec (10 Hz, 600 frame, copy + seq)"
run shm_ring/run.sh spec 10 600 --expect-torn zero --max-copy-ms 0.5
step "Spike 2b: đối chứng naive (không copy), 10 Hz, 300 frame, phải thấy frame xé"
run shm_ring/run.sh naive 10 300 --naive --expect-torn nonzero
step "Spike 2c: stress 2000 Hz, KHÔNG kiểm seq, phải thấy frame xé"
run shm_ring/run.sh stress-noseq 2000 20000 --max-sleep 0 --no-seqcheck --expect-torn nonzero
step "Spike 2d: stress 2000 Hz, có kiểm seq, phải 0 frame xé"
run shm_ring/run.sh stress-seq 2000 20000 --max-sleep 0 --expect-torn zero

echo; echo "=== Tổng kết: $FAILS bước FAIL"
exit $((FAILS > 0))
