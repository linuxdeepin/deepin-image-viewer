// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

Rectangle {
    property bool animationShow: true
    property string iconName: "deepin-image-viewer"

    anchors.top: window.top
    height: IV.GStatus.titleHeight
    visible: !window.isFullScreen
    width: window.width

    // color: titlecontrol.ColorSelector.backgroundColor
    gradient: Gradient {
        GradientStop {
            color: titlecontrol.ColorSelector.backgroundColor1
            position: 0.0
        }

        GradientStop {
            color: titlecontrol.ColorSelector.backgroundColor2
            position: 1.0
        }
    }
    Behavior on y {
        enabled: visible

        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    onAnimationShowChanged: {
        y = animationShow ? 0 : -IV.GStatus.titleHeight;
    }

    Control {
        id: titlecontrol

        property Palette backgroundColor1: Palette {
            normal: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.6)
            normalDark: Qt.rgba(26 / 255, 26 / 255, 26 / 255, 0.6)
        }
        property Palette backgroundColor2: Palette {
            normal: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.02)
            normalDark: Qt.rgba(26 / 255, 26 / 255, 26 / 255, 0.02)
        }

        hoverEnabled: true // 开启 Hover 属性
    }

    ActionButton {
        anchors.left: parent.left
        anchors.leftMargin: IV.GStatus.actionMargin
        anchors.top: parent.top
        anchors.topMargin: IV.GStatus.actionMargin

        icon {
            height: 32
            name: iconName
            width: 32
        }
    }

    // 捕获标题栏部分鼠标事件，部分事件将穿透，由底层 ApplicationWindow 处理
    IV.MouseTrackItem {
        id: trackItem

        anchors.fill: parent

        onDoubleClicked: {
            // 切换窗口最大化状态
            title.toggleWindowState();
        }
        onPressedChanged: {
            // 点击标题栏时屏蔽动画计算效果
            IV.GStatus.animationBlock = pressed;
        }
    }

    TitleBar {
        id: title

        anchors.fill: parent

        // 使用自定的文本
        content: Loader {
            active: true

            sourceComponent: Label {
                // 自动隐藏多余文本
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                text: title.title
                textFormat: Text.PlainText
                verticalAlignment: Text.AlignVCenter
            }
        }
        menu: Menu {
            onVisibleChanged: {
                IV.GStatus.animationBlock = visible;
            }

            // 打开图片动作项
            Action {
                id: openImageAction

                text: qsTr("Open image")

                onTriggered: {
                    // 发送打开窗口信号
                    stackView.openImageDialog();
                }
            }

            MenuSeparator {
            }

            ThemeMenu {
            }

            MenuSeparator {
            }

            HelpAction {
            }

            AboutAction {
                aboutDialog: AboutDialog {
                    description: qsTr("Image Viewer is an image viewing tool with fashion interface and smooth performance.")
                    productIcon: "deepin-image-viewer"
                    productName: qsTr("Image Viewer")
                    version: Qt.application.version
                    websiteLink: "www.chinauos.com"
                    websiteName: "www.chinauos.com"
                }
            }

            QuitAction {
            }
        }
    }
}
