// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.deepin.dtk 1.0

DialogWindow {
    id: dialog

    Accessible.name: "PermanentlyDelete"
    Accessible.role: Accessible.Dialog

    property alias fileName: textMetics.text
    property int nameMaxWidth: 200

    // 操作结束 true: 允许删除 false: 取消
    signal finished(bool ret)

    flags: Qt.Dialog | Qt.WindowCloseButtonHint
    maximumWidth: 400
    minimumWidth: 400
    modality: Qt.ApplicationModal
    visible: false
    width: 400

    header: DialogTitleBar {
        enableInWindowBlendBlur: false

        // 仅保留默认状态，否则 hover 上会有变化效果
        icon {
            mode: DTK.NormalState
            name: "deepin-image-viewer"
        }
    }

    Component.onCompleted: {
        show();
    }
    onVisibleChanged: {
        if (visible) {
            setX(window.x + window.width / 2 - width / 2);
            setY(window.y + window.height / 2 - height / 2);
        }
    }
    onClosing: {
        finished(false);
    }

    ColumnLayout {
        width: parent.width

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.topMargin: 8
            Layout.bottomMargin: 16
            spacing: 4

            Label {
                id: notifyText

                property Palette textColor: Palette {
                    normal: ("black")
                    normalDark: ("white")
                }

                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                color: ColorSelector.textColor
                font: DTK.fontManager.t5
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Cannot move \"%1\" to the trash. Do you want to permanently delete it?").arg(textMetics.elidedText)
                wrapMode: Text.WordWrap

                TextMetrics {
                    id: textMetics

                    elide: Text.ElideMiddle
                    elideWidth: nameMaxWidth
                    font: notifyText.font
                }
            }

            Label {
                id: messageText

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("This action cannot be undone")
                wrapMode: Text.WordWrap
            }
        }

        // uos-design: allow-manual-dialog-action-row
        // DTK DialogButtonBox reorders dynamic roles on Linux and cannot evenly
        // split the footer width, so this dialog mirrors ReName.qml's layout:
        // two half-width buttons centered with 10px margins and gap.
        Row {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
            spacing: 10

            Button {
                Accessible.name: text
                Accessible.role: Accessible.Button
                height: 36
                text: qsTr("Cancel")
                width: 185

                onClicked: {
                    finished(false);
                }
            }

            WarningButton {
                Accessible.name: text
                Accessible.role: Accessible.Button
                height: 36
                text: qsTr("Delete")
                width: 185

                onClicked: {
                    finished(true);
                }
            }
        }
    }
}
