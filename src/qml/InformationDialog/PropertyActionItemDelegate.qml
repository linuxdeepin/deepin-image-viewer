// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Control {
    id: control

    property string title
    property string description
    property int corners: RoundRectangle.NoneCorner
    property string iconName
    property Component action: ActionButton {
        visible: showPicLabel.visible
        Layout.alignment: Qt.AlignRight
        icon {
            width: 14
            height: 14
            name: control.iconName
        }

        onClicked: {
            dealShowPicLabelClick()
        }
    }

    signal clicked()

    function dealShowPicLabelClick() {
        if (showPicLabel.visible) {
            showPicLabel.visible = false
            // 每次显示编辑框时显示为图片名称
            nameedit.text = IV.FileControl.slotGetFileName(IV.GControl.currentSource)
        } else {
            var name = nameedit.text
            if (!IV.FileControl.isShowToolTip(IV.GControl.currentSource, name) && name.length > 0) {
                IV.FileControl.slotFileReName(name, IV.GControl.currentSource)
            }
            showPicLabel.visible = true
        }
    }

    // 复位当前属性编辑组件，关闭编辑框
    function reset() {
        showPicLabel.visible = true
    }

    padding: 5
    contentItem: ColumnLayout {
        Label {
            visible: control.title.length > 0
            text: control.title
            textFormat: Text.PlainText
            font: DTK.fontManager.t10
        }

        RowLayout {
            LineEdit {
                id: nameedit

                visible: !showPicLabel.visible
                text: IV.FileControl.slotGetFileName(IV.GControl.currentSource)
                anchors {
                    topMargin: 5
                    leftMargin: 10
                }
                font.pixelSize: 16
                focus: true
                selectByMouse: true
                alertText: qsTr("The file already exists, please use another name")
                showAlert: IV.FileControl.isShowToolTip(IV.GControl.currentSource, nameedit.text) && nameedit.visible
                height: 20
                // 限制输入特殊字符
                validator: RegExpValidator {
                    regExp: /^[^ \\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Enter
                            || event.key === Qt.Key_Return) {
                        dealShowPicLabelClick()
                    }
                }
            }
            Label {
                id: showPicLabel
                visible: control.description
                Layout.fillWidth: true
                text: control.description
                font: DTK.fontManager.t8
                elide: Text.ElideMiddle
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    AlertToolTip {
                        id: tip
                        parent: parent
                        visible: parent.focus
                        text: control.description
                        y: showPicLabel.y + 20
                    }

                    onMouseXChanged: {
                        if (tip.width < control.width + 15) {
                            tip.visible = false
                        } else {
                            tip.visible = true
                        }
                    }

                    onExited: {
                        tip.visible = false
                    }
                }
            }

            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }

    background: RoundRectangle {
        implicitWidth: 66
        implicitHeight: 40
        color: Qt.rgba(0, 0, 0, 0.05)
        radius: Style.control.radius
        corners: control.corners
    }
}
