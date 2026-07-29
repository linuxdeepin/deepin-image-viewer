#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
IMAGE_PATH="${PROJECT_ROOT}/src/assets/deepin-image-viewer/image-viewer/zh_CN/fig/main.png"

VIEWER_BIN="${VIEWER_BIN:-deepin-image-viewer}"
"${VIEWER_BIN}" &

sleep 3
qdbus com.deepin.imageViewer / com.deepin.imageViewer.openImageFile "${IMAGE_PATH}" || true
sleep 1
xdotool mousemove 760 820 || true
sleep 2
wait
