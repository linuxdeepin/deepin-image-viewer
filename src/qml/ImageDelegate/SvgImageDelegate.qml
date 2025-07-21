// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV

import "../Utils"

BaseImageDelegate {
    id: delegate

    // SVG 优化的实际分辨率，延迟更新以避免频繁重新栅格化
    // 优先使用图片的实际分辨率，如果尚未加载则使用视图尺寸
    property size optimizedSourceSize: {
        var actualWidth = targetImageInfo.width
        var actualHeight = targetImageInfo.height

        if (actualWidth > 0 && actualHeight > 0) {
            return Qt.size(Math.max(delegate.width, actualWidth), Math.max(delegate.height, actualHeight))
        } else {
            return Qt.size(Math.max(delegate.width, width), Math.max(delegate.height, height))
        }
    }

    status: image.status
    targetImage: image
    inputHandler: imageInput

    Image {
        id: image

        height: delegate.height
        width: delegate.width
        asynchronous: true
        cache: false
        clip: true
        fillMode: Image.PreserveAspectFit
        mipmap: true
        smooth: true
        scale: 1.0
        source: delegate.source
        // 使用延迟更新的优化分辨率，避免缩放过程中的卡顿
        sourceSize: delegate.optimizedSourceSize

        // 监听缩放变化，延迟更新分辨率
        onScaleChanged: {
            sourceSizeUpdateTimer.restart()
        }
    }

    // 监听图片信息变化，当实际分辨率可用时更新 sourceSize
    Connections {
        target: targetImageInfo

        function onWidthChanged() {
            if (targetImageInfo.width > 0) {
                sourceSizeUpdateTimer.restart()
            }
        }

        function onHeightChanged() {
            if (targetImageInfo.height > 0) {
                sourceSizeUpdateTimer.restart()
            }
        }
    }

    // 延迟更新 sourceSize 的定时器，避免频繁重新栅格化
    Timer {
        id: sourceSizeUpdateTimer
        interval: 150 // 150ms 延迟，平衡响应性和性能
        repeat: false

        onTriggered: {
            if (!delegate.source) {
                delegate.optimizedSourceSize = Qt.size(0, 0)
                return
            }

            // 使用图片的实际分辨率，而不是视图尺寸
            var actualWidth = targetImageInfo.width
            var actualHeight = targetImageInfo.height
            var currentScale = image.scale

            // 如果图片信息尚未加载完成，使用视图尺寸作为备选
            if (actualWidth <= 0 || actualHeight <= 0) {
                actualWidth = image.width
                actualHeight = image.height
            }

            // 使用正确的缩放比例计算公式：(paintedWidth * scale) / originalWidth
            // 这与 ImageViewer.qml 中的 readableScale 计算保持一致
            var actualScaleRatio = 1.0
            if (actualWidth > 0 && image.paintedWidth > 0) {
                actualScaleRatio = (image.paintedWidth * currentScale) / actualWidth
            }

            // 计算基于正确缩放比例的期望渲染分辨率
            var desiredWidth = actualWidth * actualScaleRatio
            var desiredHeight = actualHeight * actualScaleRatio

            // 保持宽高比的前提下应用最大分辨率限制，分辨率太高会导致渲染失败
            var maxDimension = 8000
            var aspectRatio = actualWidth / actualHeight

            var targetWidth, targetHeight

            if (desiredWidth <= maxDimension && desiredHeight <= maxDimension) {
                // 期望分辨率都在限制范围内，直接使用
                targetWidth = Math.max(1, desiredWidth)
                targetHeight = Math.max(1, desiredHeight)
            } else {
                // 需要按比例缩放以适应最大限制
                if (desiredWidth > desiredHeight) {
                    // 宽度是限制因素
                    targetWidth = maxDimension
                    targetHeight = Math.max(1, maxDimension / aspectRatio)
                } else {
                    // 高度是限制因素
                    targetHeight = maxDimension
                    targetWidth = Math.max(1, maxDimension * aspectRatio)
                }
            }

            delegate.optimizedSourceSize = Qt.size(Math.round(targetWidth), Math.round(targetHeight))
        }
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
    }
}
