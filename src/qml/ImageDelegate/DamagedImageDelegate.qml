// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as DTK

BaseImageDelegate {
    id: delegate

    status: Image.Error

    DTK.ActionButton {
        anchors.centerIn: parent

        icon {
            name: "photo_breach"
            width: 151
            height: 151
        }
    }
}
