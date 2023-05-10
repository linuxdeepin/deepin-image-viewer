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
    property string iconName: "deepin-image-viewer"
    property bool animationShow: true

    onAnimationShowChanged: {
        y = animationShow ? 0 : -GStatus.titleHeight
    }

    anchors.top: window.top
    width: window.width
    height: GStatus.titleHeight
    visible: !window.isFullScreen
    color: titlecontrol.ColorSelector.backgroundColor
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: titlecontrol.ColorSelector.backgroundColor1
        }
        GradientStop {
            position: 1.0
            color: titlecontrol.ColorSelector.backgroundColor2
        }
    }

    Control {
        id: titlecontrol
        hoverEnabled: true // 开启 Hover 属性
        property Palette backgroundColor1: Palette {
            normal: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.6)
            normalDark: Qt.rgba(26 / 255, 26 / 255, 26 / 255, 0.6)
        }
        property Palette backgroundColor2: Palette {
            normal: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.02)
            normalDark: Qt.rgba(26 / 255, 26 / 255, 26 / 255, 0.02)
        }
    }

    ActionButton {
        anchors.top: parent.top
        anchors.topMargin: GStatus.actionMargin
        anchors.left: parent.left
        anchors.leftMargin: GStatus.actionMargin
        icon {
            name: iconName
            width: 32
            height: 32
        }
    }

    // 捕获标题栏部分鼠标事件，部分事件将穿透，由底层 ApplicationWindow 处理
    IV.MouseTrackItem {
        id: trackItem
        anchors.fill: parent

        onPressedChanged: {
            // 点击标题栏时屏蔽动画计算效果
            GStatus.animationBlock = pressed
        }

        onDoubleClicked: {
            // 切换窗口最大化状态
            title.toggleWindowState()
        }
    }

    TitleBar {
        id: title
        anchors.fill: parent

        menu: Menu {
            // 打开图片动作项
            Action {
                id: openImageAction

                text: qsTr("Open image")
                onTriggered: {
                    // 发送打开窗口信号
                    stackView.openImageDialog()
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
                    width: 360
                    height: 362
                    productName: qsTr("Image Viewer")
                    productIcon: "deepin-image-viewer"
                    version: qsTr("Version") + ": %1".arg(Qt.application.version)
                    description: qsTr("Image Viewer is an image viewing tool with fashion interface and smooth performance.")
                    license: qsTr("%1 is released under %2").arg(productName).arg("GPLV3")
                    companyLogo: fileControl.getCompanyLogo()
                    websiteName: DTK.deepinWebsiteName
                    websiteLink: DTK.deepinWebsiteLink
                }
            }
            QuitAction {
            }

            onVisibleChanged: {
                GStatus.animationBlock = visible
            }
        }

        // 使用自定的文本
        content: Loader {
            active: true

            sourceComponent: Label {
                textFormat: Text.PlainText
                text: title.title
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                // 自动隐藏多余文本
                elide: Text.ElideRight
            }
        }
    }

    Behavior on y {
        enabled: visible
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }
}
