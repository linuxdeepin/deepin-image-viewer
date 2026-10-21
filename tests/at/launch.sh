#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
IMAGE_TEMPLATE="${PROJECT_ROOT}/src/assets/deepin-image-viewer/image-viewer/zh_CN/fig/main.png"
RUNTIME_DIR="$(mktemp -d --tmpdir deepin-image-viewer-at.XXXXXX)"
IMAGE_PATH="${RUNTIME_DIR}/main.png"

cleanup()
{
    :
}
trap cleanup EXIT INT TERM

cp -- "${IMAGE_TEMPLATE}" "${IMAGE_PATH}"
cp -- "${IMAGE_TEMPLATE}" "${RUNTIME_DIR}/next.png"

# Kill stale viewer instances left by previous suites (safe: only matches
# viewers launched from an at runtime temp dir)
pkill -9 -f "deepin-image-viewer .*deepin-image-viewer-at\." 2>/dev/null || true
sleep 0.5

VIEWER_BIN="${VIEWER_BIN:-deepin-image-viewer}"
LOG_FILE="${RUNTIME_DIR}/viewer.log"
LANG=en_US.UTF-8 LANGUAGE=en_US LC_ALL=en_US.UTF-8 \
XDG_CACHE_HOME="${RUNTIME_DIR}/cache" XDG_CONFIG_HOME="${RUNTIME_DIR}/config" \
    "${VIEWER_BIN}" "${IMAGE_PATH}" &>"${LOG_FILE}" &
VIEWER_PID=$!
disown "${VIEWER_PID}" 2>/dev/null || true
echo "${VIEWER_PID}" > "${RUNTIME_DIR}/viewer.pid"

sleep 4
xdotool mousemove 760 820 || true
sleep 2
