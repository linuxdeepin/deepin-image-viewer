// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

Item {
    id: thumbnailDelegate

    property var source
    property int imgRadius: 3
    property bool isCurrentItem: parent.ListView.isCurrentItem
    property alias shader: enterShader

    width: 30
    height: 40
    y: 20

    Rectangle {
        id: enterShader

        height: parent.height + (2 * imgRadius)
        width: parent.width + (2 * imgRadius)
        anchors {
            top: parent.top
            topMargin: -imgRadius
            left: parent.left
            leftMargin: -imgRadius
        }
        radius: imgRadius * 2
        color: "transparent"
        border.color: "#0081FF"
        border.width: imgRadius
        visible: false
    }

    transitions: Transition {
        reversible: true

        NumberAnimation {
            properties: "scale, x, width, height"
            // 调整不同宽度下的动画时间，最多310ms
            duration: width < 200 ? 100 : width / 2
            easing.type: Easing.OutInQuad
        }
    }
}
