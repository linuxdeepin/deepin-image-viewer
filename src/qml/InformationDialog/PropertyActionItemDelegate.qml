// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import org.deepin.image.viewer 1.0 as IV

Control {
    id: control

    property Component action: ActionButton {
        Layout.alignment: Qt.AlignRight
        visible: showPicLabel.visible

        onClicked: {
            dealShowPicLabelClick();
        }

        icon {
            height: 14
            name: control.iconName
            width: 14
        }
    }
    property int corners: RoundRectangle.NoneCorner
    property string description
    property int descriptionWidth: control.width - leftPadding - rightPadding - 10
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

    function dealShowPicLabelClick() {
        if (showPicLabel.visible) {
            showPicLabel.visible = false;
            // 每次显示编辑框时显示为图片名称
            nameedit.text = IV.FileControl.slotGetFileName(IV.GControl.currentSource);
        } else {
            var name = nameedit.text;
            if (!IV.FileControl.isShowToolTip(IV.GControl.currentSource, name) && name.length > 0) {
                IV.FileControl.slotFileReName(name, IV.GControl.currentSource);
            }
            showPicLabel.visible = true;
        }
    }

    // 复位当前属性编辑组件，关闭编辑框
    function reset() {
        showPicLabel.visible = true;
    }

    bottomPadding: 4
    leftPadding: 10
    rightPadding: 10
    topPadding: 3

    background: RoundRectangle {
        color: Qt.rgba(0, 0, 0, 0.05)
        corners: control.corners
        implicitHeight: 40
        implicitWidth: 66
        radius: Style.control.radius
    }
    contentItem: ColumnLayout {
        spacing: 0

        Label {
            color: control.ColorSelector.sectionTextColor
            font: DTK.fontManager.t10
            text: control.title
            textFormat: Text.PlainText
            visible: control.title.length > 0
        }

        RowLayout {
            LineEdit {
                id: nameedit

                alertText: qsTr("The file already exists, please use another name")
                color: control.ColorSelector.infoTextColor
                focus: true
                font.pixelSize: 16
                height: 20
                selectByMouse: true
                showAlert: IV.FileControl.isShowToolTip(IV.GControl.currentSource, nameedit.text) && nameedit.visible
                text: IV.FileControl.slotGetFileName(IV.GControl.currentSource)
                visible: !showPicLabel.visible

                // 限制输入特殊字符
                validator: RegExpValidator {
                    regExp: /^[^ \\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                        dealShowPicLabelClick();
                    }
                }

                anchors {
                    leftMargin: 10
                    topMargin: 5
                }
            }

            Label {
                id: showPicLabel

                Layout.fillWidth: true
                color: control.ColorSelector.infoTextColor
                font: DTK.fontManager.t8
                text: textMetics.elidedText
                visible: control.description

                TextMetrics {
                    id: textMetics

                    elide: Text.ElideMiddle
                    elideWidth: descriptionWidth
                    font: showPicLabel.font
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

                            parent: parent
                            text: control.description
                            visible: parent.focus
                            width: control.width - 5
                            y: showPicLabel.y + 20

                            background: FloatingPanel {
                                ColorSelector.family: Palette.CrystalColor
                                implicitHeight: DS.Style.toolTip.height
                                implicitWidth: 0
                                radius: DS.Style.control.radius

                                backgroundColor: Palette {
                                    normal {
                                        common: "#f0f0f0"
                                        crystal: Qt.rgba(0.20, 0.2, 0.2, 0.1)
                                    }
                                }
                            }
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
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }
}
