// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.1
import QtQml 2.11

//浮动提示框定义
Item {
    id: root
    visible: false

    property string displayStr: ""

    Rectangle {
        color: "#EEEEEE"
        radius: 20
        width: 90
        height: 45

        Text {
            anchors.centerIn: parent
            text: displayStr
        }
    }
}
