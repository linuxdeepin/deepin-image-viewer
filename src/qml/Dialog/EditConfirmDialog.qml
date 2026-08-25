// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.deepin.dtk 1.0 as DTK

DTK.DialogWindow {
    id: dialog

    property var actions: []
    property string message: ""
    property var parentWindow: null
    property string secondaryMessage: ""
    readonly property int designHeight: actions.length > 2 ? 160 : 140

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
    DTK.DWindow.enableBlurWindow: true
    DTK.DWindow.windowRadius: 18
    height: designHeight
    maximumHeight: designHeight
    maximumWidth: 400
    minimumHeight: designHeight
    minimumWidth: 400
    leftPadding: 0
    modality: Qt.ApplicationModal
    rightPadding: 0
    visible: false
    width: 400

    header: DTK.DialogTitleBar {
        enableInWindowBlendBlur: true
        icon.mode: DTK.DTK.NormalState
        icon.name: "deepin-image-viewer"
        content: Item { }
    }

    onVisibleChanged: {
        if (visible)
            centerInParent();
    }

    Item {
        height: dialog.designHeight - 50
        width: parent.width

        Item {
            id: textBlock

            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.right: parent.right
            anchors.rightMargin: 24
            anchors.top: parent.top
            height: 40

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                height: dialog.secondaryMessage === "" ? 24 : 22
                color: dialog.palette.windowText
                font.family: DTK.DTK.fontManager.t5.family
                font.pixelSize: 16
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                text: dialog.message
                verticalAlignment: Text.AlignVCenter
                width: 308.42
                wrapMode: Text.WordWrap
                y: dialog.secondaryMessage === "" ? (parent.height - height) / 2 : 0
            }

            Label {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 18
                color: dialog.palette.windowText
                font: DTK.DTK.fontManager.t7
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.7
                text: dialog.secondaryMessage
                verticalAlignment: Text.AlignVCenter
                visible: text !== ""
                wrapMode: Text.WordWrap
            }
        }

        // uos-design: allow-manual-dialog-action-row
        // DTK DialogButtonBox reorders dynamic roles and its ListView clips the third equal-width button.
        Row {
            id: actionRow

            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            height: 36
            spacing: 10

            Repeater {
                model: dialog.actions

                DTK.Button {
                    required property var modelData

                    height: 36
                    highlighted: Boolean(modelData.recommended)
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    text: modelData.text
                    width: (actionRow.width - actionRow.spacing * (dialog.actions.length - 1))
                           / dialog.actions.length

                    Accessible.name: text
                    Accessible.role: Accessible.Button

                    onClicked: {
                        dialog.actionTriggered(modelData.action);
                        dialog.close();
                    }
                }
            }
        }
    }
}
