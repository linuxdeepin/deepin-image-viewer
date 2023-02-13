// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Rectangle {
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

    property string iconName: "deepin-image-viewer"

    anchors.top: root.top
    width: parent.width
    height: 50
    visible: root.visibility === 5 ? false : true
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

    ActionButton {
        anchors.top: parent.top
        anchors.topMargin: global.actionMargin
        anchors.left: parent.left
        anchors.leftMargin: global.actionMargin
        icon {
            name: iconName
            width: 32
            height: 32
        }
    }

    MouseArea {
        //为窗口添加鼠标事件
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton //只处理鼠标左键
        property point clickPos: "0,0"
        onPressed: {
            //接收鼠标按下事件
            clickPos = Qt.point(mouse.x, mouse.y)
            sigTitlePress()
        }
        onPositionChanged: {
            //鼠标按下后改变位置
            //鼠标偏移量
            var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)

            //如果mainwindow继承自QWidget,用setPos
            root.setX(root.x + delta.x)
            root.setY(root.y + delta.y)
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

            MenuSeparator { }
            ThemeMenu { }
            MenuSeparator { }
            HelpAction { }
            AboutAction {
                aboutDialog: AboutDialog {
                    maximumWidth: 360
                    maximumHeight: 362
                    minimumWidth: 360
                    minimumHeight: 362
                    productName: qsTr("Image Viewer")
                    productIcon: "deepin-image-viewer"
                    version: qsTr("Version") + ": %1".arg(
                                 Qt.application.version)
                    description: qsTr("Image Viewer is an image viewing tool with fashion interface and smooth performance.")
                    license: qsTr("%1 is released under %2").arg(
                                 productName).arg("GPLV3")
                    companyLogo: fileControl.getCompanyLogo()
                    websiteName: DTK.deepinWebsiteName
                    websiteLink: DTK.deepinWebsiteLink
                }
            }
            QuitAction { }

            onVisibleChanged: {
                global.animationBlock = visible
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
}
