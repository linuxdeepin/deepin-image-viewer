// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.10
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

DialogWindow {
    id: renamedialog

    property string filesuffix: ".jpg"

    function setFileName(name) {
        nameedit.text = name;
    }

    function setFileSuffix(suffix) {
        filesuffix = suffix;
    }

    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    height: 180
    icon: "deepin-image-viewer"
    maximumHeight: 180
    maximumWidth: 400
    minimumHeight: 180
    minimumWidth: 400
    modality: Qt.WindowModal
    visible: false
    width: 400

    onVisibleChanged: {
        if (visible) {
            setFileName(IV.FileControl.slotGetFileName(IV.GControl.currentSource));
            setFileSuffix(IV.FileControl.slotFileSuffix(IV.GControl.currentSource));
            setX(window.x + window.width / 2 - width / 2);
            setY(window.y + window.height / 2 - height / 2);
        }
    }

    Text {
        id: renametitle

        font.pixelSize: 16
        height: 24
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Input a new name")
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignBottom
        width: 308

        anchors {
            left: parent.left
            leftMargin: 46
            top: parent.top
        }
    }

    LineEdit {
        id: nameedit

        alertText: qsTr("The file already exists, please use another name")
        focus: true
        font: DTK.fontManager.t5
        height: 36
        maximumLength: 255 - filesuffix.length
        selectByMouse: true
        showAlert: IV.FileControl.isShowToolTip(IV.GControl.currentSource, nameedit.text)
        width: 380

        validator: RegExpValidator {
            regExp: /^[^ \\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/
        }

        anchors {
            top: renametitle.bottom
            topMargin: 16
        }
    }

    Button {
        id: cancelbtn

        font.pixelSize: 16
        height: 36
        text: qsTr("Cancel")
        width: 185

        onClicked: {
            renamedialog.visible = false;
        }

        anchors {
            right: nameedit.right
            top: nameedit.bottom
            topMargin: 10
        }
    }

    Button {
        id: enterbtn

        enabled: !IV.FileControl.isShowToolTip(IV.GControl.currentSource, nameedit.text) && nameedit.text.length > 0
        height: 36
        text: qsTr("Confirm")
        width: 185

        onClicked: {
            IV.FileControl.slotFileReName(nameedit.text, IV.GControl.currentSource);
            renamedialog.visible = false;
        }

        anchors {
            left: nameedit.left
            top: nameedit.bottom
            topMargin: 10
        }
    }
}
