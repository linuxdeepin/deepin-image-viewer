// SPDX-FileCopyrightText: 2023~2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

// Dynamic sourceSize optimization during zoom.
// Two upgrade modes:
//   - Debounced (scroll wheel): Timer-delayed upgrade with retainWhileLoading
//   - Immediate (imageInfo loaded while zoomed): instant upgrade with snapshot transition
// Once upgraded, high-res sourceSize is kept until the image source changes.
Item {
    id: optimizer

    property Image targetImage: null
    property var imageInfo: null
    property real delegateWidth: 0
    property real delegateHeight: 0
    property int maxDimension: 15000
    // 225M pixels * 4 bytes = ~900MB GPU memory cap
    property int maxTotalPixels: 225 * 1000 * 1000
    // Skip upgrade if original pixel count is below this factor of display pixels.
    property real upgradeThreshold: 2.0
    // Debounce delay (ms) for scroll-wheel zoom before upgrading sourceSize.
    property int debounceInterval: 400

    property real zoomedWidth: 0
    property real zoomedHeight: 0
    readonly property bool isUpgraded: zoomedWidth > 0
    // True when upgrade was triggered by imageInfo change; consumer should use
    // snapshot transition instead of retainWhileLoading.
    readonly property bool immediateUpgrade: immediateUpgradeFlag
    property bool immediateUpgradeFlag: false
    // Convenience: true during Loading after an immediate upgrade; consumer can
    // bind this to a snapshot Loader's active property.
    readonly property bool showUpgradeSnapshot: immediateUpgradeFlag && targetImage && targetImage.status === Image.Loading

    property size optimizedSourceSize: zoomedWidth > 0 ? Qt.size(zoomedWidth, zoomedHeight) : Qt.size(delegateWidth, delegateHeight)

    function requestUpdate() {
        if (!optimizer.targetImage || optimizer.targetImage.scale <= 1.0) {
            sourceSizeUpdateTimer.stop()
            return
        }

        if (zoomedWidth > 0) {
            return
        }

        sourceSizeUpdateTimer.restart()
    }

    function applyUpgrade() {
        if (zoomedWidth > 0) {
            return
        }

        var actualWidth = optimizer.imageInfo ? optimizer.imageInfo.width : 0
        var actualHeight = optimizer.imageInfo ? optimizer.imageInfo.height : 0

        if (actualWidth <= 0 || actualHeight <= 0) {
            actualWidth = optimizer.targetImage.width
            actualHeight = optimizer.targetImage.height
        }

        if (actualWidth <= 0 || actualHeight <= 0) {
            return
        }

        if (actualWidth * actualHeight <= optimizer.delegateWidth * optimizer.delegateHeight * optimizer.upgradeThreshold) {
            return
        }

        var w = actualWidth
        var h = actualHeight
        var ratio = actualWidth / actualHeight

        if (w > optimizer.maxDimension || h > optimizer.maxDimension) {
            if (w >= h) {
                w = optimizer.maxDimension
                h = Math.max(1, Math.round(optimizer.maxDimension / ratio))
            } else {
                h = optimizer.maxDimension
                w = Math.max(1, Math.round(optimizer.maxDimension * ratio))
            }
        }

        if (optimizer.maxTotalPixels > 0) {
            var totalPixels = w * h
            if (totalPixels > optimizer.maxTotalPixels) {
                var shrinkFactor = Math.sqrt(optimizer.maxTotalPixels / totalPixels)
                w = Math.max(1, Math.round(w * shrinkFactor))
                h = Math.max(1, Math.round(h * shrinkFactor))
            }
        }

        optimizer.zoomedWidth = w
        optimizer.zoomedHeight = h
    }

    Connections {
        target: optimizer.imageInfo
        ignoreUnknownSignals: true

        function onWidthChanged() { tryApplyImmediate() }
        function onHeightChanged() { tryApplyImmediate() }

        function tryApplyImmediate() {
            if (optimizer.imageInfo.width > 0 && optimizer.imageInfo.height > 0
                    && optimizer.targetImage && optimizer.targetImage.scale > 1.0) {
                immediateUpgradeFlag = true
                applyUpgrade()
            }
        }
    }

    Timer {
        id: sourceSizeUpdateTimer
        interval: optimizer.debounceInterval
        repeat: false

        onTriggered: {
            if (!optimizer.targetImage || !optimizer.targetImage.source) {
                optimizer.zoomedWidth = 0
                optimizer.zoomedHeight = 0
                return
            }

            if (optimizer.targetImage.scale <= 1.0) {
                return
            }

            if (zoomedWidth <= 0) {
                immediateUpgradeFlag = false
                applyUpgrade()
            }
        }
    }
}
