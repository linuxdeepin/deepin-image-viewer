// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import "../"

Control {
    id: control

    property Component action: ActionButton {
        Layout.alignment: Qt.AlignRight
        visible: control.iconName

        onClicked: control.clicked()

        icon {
            height: 14
            name: control.iconName
            width: 14
        }
    }
    property int contrlImplicitWidth: 66
    property int contrlIntimplicitHeight: 40
    property int corners: RoundRectangle.NoneCorner
    property string description
    property int descriptionWidth: control.width - leftPadding - rightPadding
    property string iconName
    property Palette infoTextColor: Palette {
        normal: Qt.rgba(0, 0, 0, 1)
        normalDark: Qt.rgba(1, 1, 1, 1)
    }
    property Palette sectionTextColor: Palette {
        normal: Qt.rgba(0, 0, 0, 0.6)
        normalDark: Qt.rgba(1, 1, 1, 0.6)
    }
    property string title

    signal clicked

    bottomPadding: 4
    implicitWidth: contrlImplicitWidth
    leftPadding: 10
    rightPadding: 10
    topPadding: 3

    background: RoundRectangle {
        color: Qt.rgba(0, 0, 0, 0.05)
        corners: control.corners
        implicitHeight: contrlIntimplicitHeight
        implicitWidth: contrlImplicitWidth
        radius: Style.control.radius
    }
    contentItem: ColumnLayout {
        spacing: 0

        Label {
            color: control.ColorSelector.sectionTextColor
            font: DTK.fontManager.t10
            text: control.title
            textFormat: Text.PlainText
            visible: control.title
        }

        RowLayout {
            id: content

            Label {
                id: showlabel

                Layout.fillWidth: true
                color: control.ColorSelector.infoTextColor
                font: DTK.fontManager.t8
                text: textMetics.elidedText
                textFormat: Text.PlainText
                visible: control.description

                TextMetrics {
                    id: textMetics

                    elide: Text.ElideMiddle
                    elideWidth: descriptionWidth
                    font: showlabel.font
                    text: control.description
                }

                Loader {
                    active: textMetics.width > descriptionWidth
                    anchors.fill: parent

                    sourceComponent: MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true

                        onExited: {
                            tip.visible = false;
                        }
                        onMouseXChanged: {
                            if (textMetics.width < descriptionWidth) {
                                tip.visible = false;
                            } else {
                                tip.visible = true;
                            }
                        }

                        ToolTip {
                            id: tip

                            // 此处代码并非设置背景，而是由palette的变更信号触发 ColorSelector.controlTheme 的更新
                            palette.window: DTK.themeType === ApplicationHelper.LightType ? "white" : "black"
                            parent: parent
                            text: control.description
                            visible: parent.focus
                            width: control.width - 5
                            y: showlabel.y + 20

                            contentItem: Text {
                                color: control.palette.toolTipText
                                font: DTK.fontManager.t8
                                horizontalAlignment: Text.AlignLeft
                                text: control.description
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }

            Loader {
                Layout.rightMargin: 5
                sourceComponent: control.action
            }
        }
    }
}
