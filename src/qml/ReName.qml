// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
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

    function getFileName(name) {
        nameedit.text = name
    }

    function getFileSuffix(suffix) {
        filesuffix.text = suffix
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
        maximumLength: 255 - filesuffix.text.length
        validator: RegExpValidator {
            regExp: /^[^ \\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/
        }
        selectByMouse: true
        alertText: qsTr("The file already exists, please use another name")
        showAlert: fileControl.isShowToolTip(source, nameedit.text)
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
        enabled: !fileControl.isShowToolTip(source, nameedit.text)
                 && nameedit.text.length > 0

        onClicked: {
            var name = nameedit.text
            //bool返回值判断是否成功
            if (fileControl.slotFileReName(name, source)) {
                sourcePaths = fileControl.renameOne(sourcePaths, source, fileControl.getNamePath(source, name))
                source = fileControl.getNamePath(source, name)
            }
            renamedialog.visible = false
        }
    }

    onVisibleChanged: {
        if (visible) {
            setFileName(fileControl.slotGetFileName(imageViewer.source))
            setFileSuffix(fileControl.slotFileSuffix(imageViewer.source))
            setX(root.x + root.width / 2 - width / 2)
            setY(root.y + root.height / 2 - height / 2)
        }
    }
}
