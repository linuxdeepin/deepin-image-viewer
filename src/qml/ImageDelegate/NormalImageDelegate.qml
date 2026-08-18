// SPDX-FileCopyrightText: 2023~2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV
import "../Utils"

BaseImageDelegate {
    id: delegate

    property bool rotationRunning: false
    property bool sourceUpdatePending: false
    readonly property int neighborPreviewMaxDimension: 960
    readonly property int rapidSwitchPreviewMaxDimension: 512

    function previewSize(maxDimension) {
        if (delegate.width <= 0 || delegate.height <= 0) {
            return Qt.size(maxDimension, maxDimension)
        }

        var longestEdge = Math.max(delegate.width, delegate.height)
        if (longestEdge <= maxDimension) {
            return Qt.size(delegate.width, delegate.height)
        }

        var scale = maxDimension / longestEdge
        return Qt.size(Math.round(delegate.width * scale), Math.round(delegate.height * scale))
    }

    function neighborPreviewSize() {
        return previewSize(neighborPreviewMaxDimension)
    }

    function rapidSwitchPreviewSize() {
        return previewSize(rapidSwitchPreviewMaxDimension)
    }

    function resetSource() {
        // check if source rename
        updateSource();

        // 加载完成，触发动画效果
        var temp = image.source;
        image.source = "";
        // 重置初始状态
        delegate.inited = false;
        image.source = temp;
    }

    function updateSource() {
        sourceSizeOptimizer.resetSourceSize()
        if (delegate.source != "") {
            // Do not retain the old texture while changing images; retain it only for same-image scale upgrades.
            sourceUpdatePending = true
            // 由于会 resetSource() 破坏绑定，因此重新设置源数据
            image.source = delegate.editSource();
        } else {
            sourceUpdatePending = false
            image.source = "";
        }
    }

    function editSource() {
        if (delegate.isCurrentImage && IV.GStatus.editMode
                && IV.ImageEditor.isEditing(delegate.source, delegate.frameIndex)) {
            return "image://EditedImage/current?revision=" + IV.ImageEditor.revision;
        }
        return "image://ImageLoad/" + delegate.source + "#frame_" + delegate.frameIndex;
    }

    inputHandler: imageInput
    status: image.status
    targetImage: image

    onFrameIndexChanged: updateSource()
    onSourceChanged: updateSource()

    Image {
        id: image

        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectFit
        height: delegate.height
        mipmap: true
        scale: 1.0
        smooth: true
        source: delegate.editSource()
        sourceSize: delegate.isCurrentImage ? (delegate.rapidSwitching
                                                 ? delegate.rapidSwitchPreviewSize()
                                                 : sourceSizeOptimizer.optimizedSourceSize)
                                             : delegate.neighborPreviewSize()
        width: delegate.width
        // debounced (scroll wheel): retain old texture for smooth transition
        // immediate (large jump): no retain, use snapshot instead
        retainWhileLoading: !delegate.sourceUpdatePending && !sourceSizeOptimizer.immediateUpgrade

        onScaleChanged: {
            sourceSizeOptimizer.requestUpdate()
        }

        onStatusChanged: {
            if (Image.Ready === image.status || Image.Error === image.status) {
                sourceUpdatePending = false
            }
            if (Image.Ready === image.status && !rotationRunning) {
                rotateAnimationLoader.active = false;
                if (upgradeSnapshotLoader.active) {
                    upgradeSnapshotLoader.active = false
                }
            }
        }
    }


    Connections {
        function onActiveChanged() { delegate.updateSource() }
        function onRevisionChanged() { delegate.updateSource() }

        target: IV.ImageEditor
    }

    Connections {
        function onEditModeChanged() { delegate.updateSource() }

        target: IV.GStatus
    }

    SourceSizeOptimizer {
        id: sourceSizeOptimizer
        targetImage: image
        imageInfo: targetImageInfo
        delegateWidth: delegate.width
        delegateHeight: delegate.height
    }

    // Snapshot for immediate mode: shows old texture while new texture loads
    Loader {
        id: upgradeSnapshotLoader
        active: sourceSizeOptimizer.showUpgradeSnapshot
        anchors.fill: parent

        sourceComponent: ShaderEffectSource {
            anchors.centerIn: parent
            width: image.width
            height: image.height
            sourceItem: image
            live: false
            hideSource: true
            scale: image.scale
            mipmap: true
            smooth: true
        }
    }

    // 旋转动画效果
    Loader {
        id: rotateAnimationLoader

        active: false
        anchors.fill: parent

        sourceComponent: Item {
            id: rotateItem

            property real previousRealWidth: 0

            function calcAnimation() {
                rotationAnimation.to = IV.GControl.currentRotation;
                // 初始化缩放比后再允许动画
                imageProxy.scale = image.scale;
                imageScaleBehavior.enabled = true;

                // 代理从图片当前拖拽位置启动，使动画起始帧与真实图片重合，
                // 避免旋转开始/结束时代理（居中）与真实图片（拖拽偏移）位置不一致导致的抖动
                imageProxy.x = image.x;
                imageProxy.y = image.y;

                // 重置缓存位置为中心，确保图片重载后 resetCache() 恢复到 (0,0)，
                // 与代理动画终点 (0,0) 一致，消除动画结束时位置跳变
                delegate.targetImageInfo.x = 0;
                delegate.targetImageInfo.y = 0;

                // 记录之前的绘制宽度，用于计算缩放比例
                previousRealWidth = image.paintedWidth;

                // 触发动画
                aniamtion.start();
            }

            anchors.fill: parent

            Connections {
                function onPaintedWidthChanged() {
                    // 注意宽高交换，缩放比 = 实际显示的高度(绘制高度 * 缩放比) / 之前绘制的高度
                    // 因此缩放的图片也能正常旋转匹配
                    imageProxy.scale = (image.paintedHeight * image.scale) / rotateItem.previousRealWidth;
                }

                target: image
            }

            ShaderEffectSource {
                id: imageProxy

                // 不使用 anchors.centerIn，位置由 calcAnimation() 根据图片当前位置设置
                height: image.height
                live: false
                sourceItem: image
                width: image.width

                Behavior on scale {
                    id: imageScaleBehavior

                    enabled: false

                    NumberAnimation {
                        id: scaleAnimation

                        duration: IV.GStatus.animationDefaultDuration - delayUpdate.interval
                        easing.type: Easing.OutExpo
                    }
                }

                Component.onCompleted: {
                    scheduleUpdate();
                    // 计算动画参数并触发动画
                    calcAnimation();
                }
            }

            Timer {
                id: delayUpdate

                interval: 50

                onTriggered: {
                    delegate.resetSource();
                }
            }

            // 并行动画
            ParallelAnimation {
                id: aniamtion

                alwaysRunToEnd: true

                onRunningChanged: {
                    if (running) {
                        image.visible = false;
                        delayUpdate.start();
                    }
                    if (!running && Image.Ready === image.status) {
                        image.visible = true;
                        rotateAnimationLoader.active = false;
                    }
                    rotationRunning = running;
                }

                RotationAnimation {
                    id: rotationAnimation

                    direction: RotationAnimation.Shortest
                    duration: IV.GStatus.animationDefaultDuration
                    easing.type: Easing.OutExpo
                    target: imageProxy
                }

                NumberAnimation {
                    duration: IV.GStatus.animationDefaultDuration
                    easing.type: Easing.OutExpo
                    properties: "x, y"
                    target: imageProxy
                    to: 0
                }
            }
        }
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        isRotatable: IV.FileControl.isRotatable(delegate.source)
        targetImage: image.status === Image.Ready ? image : null
    }

    Connections {
        function onChangeRotationCacheBegin() {
            // Note: 确保缓存中的数据已刷新后更新界面
            // 0 为复位，缓存中的数据已转换，无需再次加载
            if (0 !== IV.GControl.currentRotation) {
                // 激活旋转动画加载器
                rotateAnimationLoader.active = true;
            }
        }

        enabled: isCurrentImage
        target: IV.GControl
    }
}
