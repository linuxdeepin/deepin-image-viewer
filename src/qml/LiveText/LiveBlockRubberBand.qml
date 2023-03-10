// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

Item {
    id: root

    visible: false

    function setActiveColor(color) {
        rect.color = color
    }

    Rectangle {
        id: rect
        opacity: 0.15
        anchors.fill: parent
    }

    Component.onCompleted: {
        rect.color = cursorTool.activeColor()
        cursorTool.activeColorChanged.connect(setActiveColor)
    }
}
