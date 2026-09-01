#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
IMAGE_TEMPLATE="${PROJECT_ROOT}/src/assets/deepin-image-viewer/image-viewer/zh_CN/fig/main.png"
RUNTIME_DIR="$(mktemp -d --tmpdir deepin-image-viewer-at.XXXXXX)"
IMAGE_PATH="${RUNTIME_DIR}/main.png"

cleanup()
{
    rm -rf "${RUNTIME_DIR}"
}
trap cleanup EXIT INT TERM

cp -- "${IMAGE_TEMPLATE}" "${IMAGE_PATH}"
cp -- "${IMAGE_TEMPLATE}" "${RUNTIME_DIR}/next.png"

VIEWER_BIN="${VIEWER_BIN:-deepin-image-viewer}"
LANG=en_US.UTF-8 LANGUAGE=en_US LC_ALL=en_US.UTF-8 \
XDG_CACHE_HOME="${RUNTIME_DIR}/cache" XDG_CONFIG_HOME="${RUNTIME_DIR}/config" \
    "${VIEWER_BIN}" "${IMAGE_PATH}" &
VIEWER_PID=$!

sleep 4
xdotool mousemove 760 820 || true
sleep 2
wait "${VIEWER_PID}"
