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

    property real sourceWidth: 0 // 图片源宽度
    property real sourceHeight: 0 // 图片源高度
    property real rotation: 0 // 图片旋转角度
    property real scale: 1 // 图片缩放比例

    property alias baseMouseArea: mouseArea

    // 外部创建会覆盖 Flickable 组件
    MouseArea {
        id: mouseArea

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onWheel: {

        }
    }

    // 和MouseArea存在先后顺序，使用触摸时优先处理触摸事件
    PinchArea {
        id: imagePinchArea

        // 记录旧的缩放大小，防止拖拽时未保留当前状态
        property double oldScale: 0
        property double oldRotate: 0
        property bool isRotatable: false

        enabled: isMousePinchArea
        anchors.fill: parent

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
            imageViewer.currentScale = pinch.scale * imagePinchArea.oldScale
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
