// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQml 2.11
import org.deepin.image.viewer 1.0 as IV

Item {
    id: baseDelegate

    property url source
    property int type: IV.Types.NullImage
    property int status: Image.Null
    property bool isCurrentImage: index === GControl.currentIndex

    property var targetImage
    property real sourceWidth: targetImage ? targetImage.sourceSize.width : -1 // 图片源宽度
    property real sourceHeight: targetImage ? targetImage.sourceSize.height : -1 // 图片源高度
    property real paintedWidth: targetImage ? targetImage.paintedWidth : -1
    property real paintedHeight: targetImage ? targetImage.paintedHeight: -1
    property real rotation: 0 // 图片旋转角度
    property real scale: 1 // 图片缩放比例

    property alias baseMouseArea: mouseArea

    function reset() {
        rotation = 0
        scale = 1.0

        if (undefined !== targetImage) {
            mouseArea.updateDragRect()
        }
    }

    onIsCurrentImageChanged: reset()

    ////! \test 调试使用
    onPaintedHeightChanged: {
        console.warn("------------", source, sourceWidth, paintedWidth, paintedHeight,
                     paintedWidth * scale, index)
    }

    onStatusChanged: {
        if (Image.Ready === status) {
            mouseArea.updateDragRect()
        }
    }

    // 外部创建会覆盖 Flickable 组件
    MouseArea {
        id: mouseArea

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: targetImage !== undefined
        drag.target: targetImage ? targetImage : undefined

        function updateDragRect() {
            if (undefined === baseDelegate.targetImage) {
                return
            }

            if (baseDelegate.scale <= 1.0) {
                drag.minimumX = 0
                drag.minimumY = 0
                drag.maximumX = 0
                drag.maximumY = 0
                baseDelegate.targetImage.x = 0
                baseDelegate.targetImage.y = 0

            } else {
                var pixelWidthDelta = baseDelegate.paintedWidth * baseDelegate.scale - window.width;
                drag.maximumX = pixelWidthDelta > 0 ? pixelWidthDelta / 2 : 0
                drag.minimumX = -drag.maximumX
                var pixelHeightDelta = baseDelegate.paintedHeight * baseDelegate.scale - window.height;
                drag.maximumY = pixelHeightDelta > 0 ? pixelHeightDelta / 2 : 0
                drag.minimumY = -drag.maximumY

                baseDelegate.targetImage.x = Math.max(drag.minimumX, Math.min(baseDelegate.targetImage.x, drag.maximumX))
                baseDelegate.targetImage.y = Math.max(drag.minimumY, Math.min(baseDelegate.targetImage.y, drag.maximumY))
            }
        }

        onPressed: {
            if (Qt.RightButton === mouse.button) {
                GControl.showRightMenu = true
            }
        }

        onWheel: {
            var detla = wheel.angleDelta.y / 120
            // 通过Keys缓存的状态可能不准确，在焦点移出时release事件没有正确捕获，
            // 修改为通过当前事件传入的按键按下信息判断
            if (Qt.ControlModifier & wheel.modifiers) {
                detla > 0 ? GControl.previousImage() : GControl.nextImage()
            } else {
                if (undefined === baseDelegate.targetImage) {
                    return
                }

                // 缓存当前的坐标信息
                var mapPoint = mapToItem(baseDelegate.targetImage, wheel.x, wheel.y)
                if (detla > 0) {
                    baseDelegate.scale = baseDelegate.scale / 0.9
                } else {
                    baseDelegate.scale = baseDelegate.scale * 0.9
                }

                var restorePoint = mapFromItem(baseDelegate.targetImage, mapPoint.x, mapPoint.y)
                baseDelegate.targetImage.x = baseDelegate.targetImage.x - restorePoint.x + wheel.x
                baseDelegate.targetImage.y = baseDelegate.targetImage.y - restorePoint.y + wheel.y

                updateDragRect()

                //// FIXME 通知位置变更，刷新导航窗口
            }
        }
    }

    // 和MouseArea存在先后顺序，使用触摸时优先处理触摸事件
    PinchArea {
        id: imagePinchArea

        // 记录旧的缩放大小，防止拖拽时未保留当前状态
        property double oldScale: 0
        property double oldRotate: 0
        property bool isRotatable: false

        anchors.fill: parent
        enabled: targetImage !== undefined

        onPinchStarted: {
            // 缩放和旋转都至少需要2指操作
            if (pinch.pointCount !== 2) {
                pinch.accepted = false
                return
            }

            oldScale = baseDelegate.scale
            oldRotate = baseDelegate.rotation
            // 不绑定信号，无需每次计算，仅当处理时获取
            // isRotatable = fileControl.isRotatable(imageViewer.source)
            pinch.accepted = true
        }

        onPinchUpdated: {
            // 不设置边界，通过 onCurrentScaleChanged 处理限制缩放范围在 2% ~ 2000%
            baseDelegate.scale = pinch.scale * oldScale

            if (isRotatable) {
                baseDelegate.rotation = pinch.rotation + oldRotate
            }
        }

        onPinchFinished: {
            // 更新界面缩放大小
            baseDelegate.scale = pinch.scale * imagePinchArea.oldScale
            msArea.changeRectXY()

            // 判断当前图片是否允许旋转
            if (imagePinchArea.isRotatable) {
                // 计算旋转角度，限制在旋转梯度为90度，以45度为分界点
                if (Math.abs(pinch.rotation) > 45) {
                    // 区分正反旋转方向
                    var isClockWise = pinch.rotation > 0
                    // 计算绝对角度值
                    var rotateAngle = Math.floor((Math.abs(pinch.rotation) + 45) / 90) * 90;

                    // 触摸旋转保存属于低频率操作，可立即保存文件
                    fileControl.rotateFile(imageViewer.source, isClockWise ? rotateAngle : -rotateAngle)
                    fileControl.slotRotatePixCurrent()
                } else {
                    baseDelegate.rotation = imagePinchArea.oldRotate
                }
            }
        }

        // 多点触控区域，处理触摸屏上的点击、双击、长按事件
        MultiPointTouchArea {
            id: multiPointTouchArea

            // 当前实时触摸点数
            property int currentTouchPointsCount: 0
            // 判断是否允许切换标题栏和工具栏状态
            property bool enableSwitchState: true

            anchors.fill: parent
            minimumTouchPoints: 1
            maximumTouchPoints: 3
            // 仅处理触摸事件，鼠标点击事件由MouseArea处理
            mouseEnabled: false

            // 双击动作处理
            function doubleClickProcess() {
                view.exitLiveText()
                infomationDig.hide()
                showFulltimer.start()
            }

            onReleased: {
                // 触摸状态下，单指点击需要弹出隐藏的标题栏和工具栏(即立即显示)
                if (touchPoints.length === 1 && !menuHideTimer.running) {
                    if (enableSwitchState) {
                        // 还需要考虑全屏下处理
                        fullThumbnail.switchTopAndBottomBarState();
                    }
                    // 复位状态
                    enableSwitchState = true

                    // 进行双击动作判断
                    if (doubleClicekdTimer.running) {
                        doubleClickProcess()
                        doubleClicekdTimer.stop()
                    } else {
                        doubleClicekdTimer.restart()
                    }
                }
            }

            // 获取当前实时触摸点数
            onTouchUpdated: {
                currentTouchPointsCount = touchPoints.length
            }

            // 用于记录菜单隐藏的定时器
            Timer {
                id: menuHideTimer

                running: !option_menu.visible
                interval: 400
            }

            // 双击触发定时器 双击间隔400ms内触发全屏展示
            Timer {
                id: doubleClicekdTimer

                interval: 400
            }

            // 长按触发定时器
            Timer {
                id: pressHoldTimer

                // 仅一个触点按下时，延时400ms触发右键菜单
                running: multiPointTouchArea.currentTouchPointsCount === 1
                interval: 400

                onTriggered: {
                    infomationDig.hide()
                    multiPointTouchArea.enableSwitchState = false
                    // 弹出右键菜单
                    option_menu.popup()
                }
            }
        }
    }
}
