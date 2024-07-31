// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

Item {
    id: thumbnailDelegate

    property int imgRadius: 3
    property bool isCurrentItem: parent.ListView.isCurrentItem
    property alias shader: enterShader
    property var source

    height: 40
    width: 30
    y: 20

    transitions: Transition {
        reversible: true

        // 保留最明显的宽度变更动画，其它属性变更忽略
        NumberAnimation {
            // 调整不同宽度下的动画时间，最多310ms
            duration: Math.max(width / 2, 366)
            easing.type: Easing.OutExpo
            properties: "width, height, x, y"
        }
    }

    Rectangle {
        id: enterShader

        // 如需改为匹配高亮色 palette.highlight
        border.color: "#0081FF"
        border.width: imgRadius
        color: "transparent"
        height: parent.height + (2 * imgRadius)
        radius: imgRadius * 2
        visible: false
        width: parent.width + (2 * imgRadius)

        anchors {
            left: parent.left
            leftMargin: -imgRadius
            top: parent.top
            topMargin: -imgRadius
        }
    }
}
