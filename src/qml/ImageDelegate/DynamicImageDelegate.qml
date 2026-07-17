// SPDX-FileCopyrightText: 2023~2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV
import "../Utils"

BaseImageDelegate {
    id: delegate

    property bool needInit: true
    readonly property bool imageInfoReady: targetImageInfo.status === IV.ImageInfo.Ready

    function scaledSourceSize() {
        var sourceWidth = targetImageInfo.width
        var sourceHeight = targetImageInfo.height
        var requestedSize = sourceSizeOptimizer.optimizedSourceSize

        if (sourceWidth <= 0 || sourceHeight <= 0
                || requestedSize.width <= 0 || requestedSize.height <= 0) {
            return Qt.size(0, 0)
        }

        var scale = Math.min(1,
                             requestedSize.width / sourceWidth,
                             requestedSize.height / sourceHeight)
        return Qt.size(Math.max(1, Math.round(sourceWidth * scale)),
                       Math.max(1, Math.round(sourceHeight * scale)))
    }

    inputHandler: imageInput
    status: image.status
    targetImage: image

    AnimatedImage {
        id: image

        asynchronous: true
        cache: false
        clip: true
        fillMode: Image.PreserveAspectFit
        height: delegate.height
        scale: 1.0
        smooth: true
        // 等待原始尺寸就绪，避免将显示区域的宽高直接传给动图解码器而拉伸帧内容。
        source: delegate.imageInfoReady ? delegate.source : ""
        // 限制每帧解码分辨率，同时保持原图比例且不在解码阶段放大。
        sourceSize: delegate.imageInfoReady ? delegate.scaledSourceSize() : Qt.size(0, 0)
        width: delegate.width

        onScaleChanged: {
            sourceSizeOptimizer.requestUpdate()
        }
    }

    onSourceChanged: {
        needInit = true
        sourceSizeOptimizer.resetSourceSize()
    }

    SourceSizeOptimizer {
        id: sourceSizeOptimizer
        targetImage: image
        imageInfo: targetImageInfo
        delegateWidth: delegate.width
        delegateHeight: delegate.height
        maxDimension: 4096
        maxTotalPixels: 0
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
    }

    // 动图在首次加载，状态变更为 Ready 时，paintedWidth 可能未更新，为0
    // 手动复位图片状态，调整缩放比例
    Connections {
        function onPaintedWidthChanged() {
            if (image.paintedWidth > 0) {
                needInit = false;
                delegate.reset();
            }
        }

        enabled: needInit
        target: image
    }
}
