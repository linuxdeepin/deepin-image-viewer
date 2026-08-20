// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.deepin.dtk 1.0

DialogWindow {
    id: dialog

    property var actions: []
    property string message: ""
    property var parentWindow: null
    property string secondaryMessage: ""

    signal actionTriggered(string action)

    function centerInParent() {
        const targetWindow = parentWindow || Window.window;
        if (!targetWindow)
            return;
        setX(targetWindow.x + targetWindow.width / 2 - width / 2);
        setY(targetWindow.y + targetWindow.height / 2 - height / 2);
    }

    function open() {
        centerInParent();
        show();
        requestActivate();
    }

    flags: Qt.Dialog | Qt.WindowCloseButtonHint
    color: palette.window
    DWindow.enableBlurWindow: false
    DWindow.windowRadius: 16
    height: 180
    maximumHeight: 180
    maximumWidth: 400
    minimumHeight: 180
    minimumWidth: 400
    modality: Qt.ApplicationModal
    visible: false
    width: 400

    header: DialogTitleBar {
        enableInWindowBlendBlur: false
        icon.mode: DTK.NormalState
        icon.name: "deepin-image-viewer"
        content: Item { }
    }

    onVisibleChanged: {
        if (visible)
            centerInParent();
    }

    Item {
        height: 130
        width: parent.width

        Item {
            id: textBlock

            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.right: parent.right
            anchors.rightMargin: 24
            anchors.top: parent.top
            anchors.topMargin: 3
            height: 46

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: dialog.secondaryMessage === "" ? parent.height : 26
                color: dialog.palette.windowText
                font: DTK.fontManager.t5
                horizontalAlignment: Text.AlignHCenter
                text: dialog.message
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }

            Label {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20
                color: dialog.palette.windowText
                font: DTK.fontManager.t7
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.7
                text: dialog.secondaryMessage
                verticalAlignment: Text.AlignVCenter
                visible: text !== ""
            }
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            height: 36
            spacing: 10

            Repeater {
                model: dialog.actions

                Item {
                    required property var modelData

                    property int buttonWidth: dialog.actions.length > 2 ? 100 : 155

                    height: 36
                    width: buttonWidth

                    Button {
                        anchors.fill: parent
                        text: parent.modelData.text
                        visible: !Boolean(parent.modelData.recommended)

                        Accessible.name: text
                        Accessible.role: Accessible.Button

                        onClicked: {
                            dialog.actionTriggered(parent.modelData.action);
                            dialog.close();
                        }
                    }

                    RecommandButton {
                        anchors.fill: parent
                        text: parent.modelData.text
                        visible: Boolean(parent.modelData.recommended)

                        Accessible.name: text
                        Accessible.role: Accessible.Button

                        onClicked: {
                            dialog.actionTriggered(parent.modelData.action);
                            dialog.close();
                        }
                    }

                }
            }
        }
    }
}
