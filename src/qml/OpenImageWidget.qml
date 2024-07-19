// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Control {
    id: control

    anchors.fill: parent

    DciIcon {
        id: openWidgetImage

        anchors.centerIn: parent
        name: "import_photo"
        sourceSize.height: 128
        sourceSize.width: 128
        theme: DTK.themeType
    }

    RecommandButton {
        id: openFileBtn

        font.capitalization: Font.MixedCase
        height: 35
        text: qsTr("Open Image")
        width: 300

        onClicked: stackView.openImageDialog()

        anchors {
            left: openWidgetImage.left
            leftMargin: -86
            top: openWidgetImage.bottom
            topMargin: 10
        }
    }
}
