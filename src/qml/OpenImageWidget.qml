// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0 as DTK

Rectangle {
    anchors.fill: parent
    color: backcontrol.DTK.ColorSelector.backgroundColor

    DTK.ActionButton {
        id: openWidgetImage

        anchors.centerIn: parent
        icon {
            name: "import_photo"
            width: 128
            height: 128
        }
    }

    DTK.RecommandButton {
        id: openFileBtn

        width: 300
        height: 35
        anchors {
            top: openWidgetImage.bottom
            topMargin: 10
            left: openWidgetImage.left
            leftMargin: -86
        }
        font.capitalization: Font.MixedCase
        text: qsTr("Open Image")

        onClicked: stackView.openImageDialog()
    }
}
