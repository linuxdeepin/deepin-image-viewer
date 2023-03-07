// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.10
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

DialogWindow {
    id: renamedialog

    property string filesuffix: ".jpg"

    visible: false
    width: 400
    height: 180
    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: 180
    maximumHeight: 180
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    icon: "deepin-image-viewer"

    function setFileName(name) {
        nameedit.text = name
    }

    function setFileSuffix(suffix) {
        filesuffix = suffix
    }

    Text {
        id: renametitle

        anchors {
            left: parent.left
            leftMargin: 46
            top: parent.top
        }
        width: 308
        height: 24
        font.pixelSize: 16
        text: qsTr("Input a new name")
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
    }

    LineEdit {
        id: nameedit

        anchors {
            top: renametitle.bottom
            topMargin: 16
        }
        width: 380
        height: 36
        font: DTK.fontManager.t5
        focus: true
        maximumLength: 255 - filesuffix.length
        validator: RegExpValidator {
            regExp: /^[^ \\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/
        }
        selectByMouse: true
        alertText: qsTr("The file already exists, please use another name")
        showAlert: fileControl.isShowToolTip(GControl.currentSource, nameedit.text)
    }

    Button {
        id: cancelbtn

        anchors {
            top: nameedit.bottom
            topMargin: 10
            right: nameedit.right
        }
        text: qsTr("Cancel")
        width: 185
        height: 36
        font.pixelSize: 16

        onClicked: {
            renamedialog.visible = false
        }
    }

    Button {
        id: enterbtn

        anchors {
            top: nameedit.bottom
            topMargin: 10
            left: nameedit.left
        }
        width: 185
        height: 36
        text: qsTr("Confirm")
        enabled: !fileControl.isShowToolTip(GControl.currentSource, nameedit.text) && nameedit.text.length > 0

        onClicked: {
            fileControl.slotFileReName(nameedit.text, GControl.currentSource);
            renamedialog.visible = false
        }
    }

    onVisibleChanged: {
        if (visible) {
            setFileName(fileControl.slotGetFileName(GControl.currentSource))
            setFileSuffix(fileControl.slotFileSuffix(GControl.currentSource))
            setX(root.x + root.width / 2 - width / 2)
            setY(root.y + root.height / 2 - height / 2)
        }
    }
}
