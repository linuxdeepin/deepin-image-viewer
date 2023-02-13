// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Item {
    id: sliderShow

    // 减少重复触发，变更后单独更新图片源
    property alias source: fadeInOutImage.imageSource
    property bool autoRun: false

    anchors.fill: parent

    function restart() {
        autoRun = true
        fadeInOutImage.restart()
    }

    function outSliderShow() {
        showNormal()
        stackView.switchImageView()
    }

    function switchNextImage() {
        if (!GControl.hasNextImage) {
            GControl.firstImage()
        } else {
            GControl.nextImage()
        }

        source = "image://multiimage/" + GControl.currentSource + "#frame_"
                + GControl.currentFrameIndex + "_thumbnail"
    }

    function switchPreviousImage() {
        if (!GControl.hasPreviousImage) {
            GControl.lastImage()
        } else {
            GControl.previousImage()
        }

        source = "image://multiimage/" + GControl.currentSource + "#frame_"
                + GControl.currentFrameIndex + "_thumbnail"
    }

    Timer {
        id: timer

        interval: 3000
        running: autoRun
        repeat: true

        onTriggered: switchNextImage()
    }

    SFadeInOut {
        id: fadeInOutImage
        anchors.fill: parent
    }

    MouseArea {
        id: sliderArea

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: "BlankCursor"
        hoverEnabled: true

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                sliderMenu.popup()
            }
        }

        onDoubleClicked: outSliderShow()
        onCursorShapeChanged: sliderCursorTimer.start()

        onMouseXChanged: {
            sliderArea.cursorShape = "ArrowCursor"
        }

        onMouseYChanged: {
            sliderArea.cursorShape = "ArrowCursor"
            if (mouseY > height - 100) {
                showSliderAnimation.start()
            } else {
                hideSliderAnimation.start()
            }
        }

        Timer {
            id: sliderCursorTimer

            interval: 3000 // 设置定时器定时时间为500ms,默认1000ms
            running: true // 是否开启定时，默认是false，当为true的时候，进入此界面就开始定时
            repeat: true // 是否重复定时,默认为false
            onTriggered: sliderArea.cursorShape = "BlankCursor"
        }

        NumberAnimation {
            id: hideSliderAnimation

            target: sliderFloatPanel
            from: sliderFloatPanel.y
            to: screen.height
            property: "y"
            duration: 200
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id: showSliderAnimation

            target: sliderFloatPanel
            from: sliderFloatPanel.y
            to: screen.height - 80
            property: "y"
            duration: 200
            easing.type: Easing.InOutQuad
        }

        FloatingPanel {
            id: sliderFloatPanel

            width: 232
            height: 70

            Component.onCompleted: {
                sliderFloatPanel.x = (screen.width - width) / 2
                sliderFloatPanel.y = screen.height - 80
            }

            Row {
                height: 50
                anchors {
                    left: parent.left
                    leftMargin: 10
                    top: parent.top
                    topMargin: parent.height / 2 - height / 2
                }
                spacing: 10

                IconButton {
                    id: sliderPrevious

                    icon.name: "icon_previous"
                    width: 50
                    height: parent.height
                    ToolTip.delay: 500
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Previous")

                    onClicked: {
                        switchPreviousImage()
                        autoRun = false
                    }
                }

                IconButton {
                    id: sliderPause

                    icon.name: autoRun ? "icon_suspend" : "icon_play_start"
                    width: 50
                    height: parent.height
                    ToolTip.delay: 500
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    ToolTip.text: autoRun ? qsTr("Pause") : qsTr("Play")

                    onClicked: {
                        autoRun = !autoRun
                    }
                }

                IconButton {
                    id: sliderNext

                    icon.name: "icon_next"
                    width: 50
                    height: parent.height
                    ToolTip.delay: 500
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Next")

                    onClicked: {
                        switchNextImage()
                        autoRun = false
                    }
                }

                ActionButton {
                    icon.name: "entry_clear"
                    width: 24
                    height: parent.height
                    ToolTip.delay: 500
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Exit")

                    onClicked: outSliderShow()
                }
            }
        }
    }

    Menu {
        id: sliderMenu

        x: 250
        y: 600

        MenuItem {
            text: autoRun ? qsTr("Pause") : qsTr("Play")
            onTriggered: {
                autoRun = !autoRun
            }

            // 添加处理快捷键，播放幻灯片时暂停/播放
            Shortcut {
                id: pauseShortCut

                sequence: "Space"
                // 进行幻灯片播放时允许响应空格快捷键处理暂停/播放
                enabled: sliderShow.visible

                onActivated: {
                    autoRun = !autoRun
                }
            }
        }

        MenuItem {
            text: qsTr("Exit")
            onTriggered: outSliderShow()

            Shortcut {
                sequence: "Esc"
                onActivated: outSliderShow()
            }
        }
    }

    Component.onCompleted: {
        source = "image://multiimage/" + GControl.currentSource + "#frame_"
                + GControl.currentFrameIndex + "_thumbnail"

        showFullScreen()
        restart()
    }
}
