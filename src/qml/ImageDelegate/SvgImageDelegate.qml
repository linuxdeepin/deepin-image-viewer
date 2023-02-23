// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

BaseImageDelegate {
    id: delegate

    status: image.status

    Image {
        id: image

        anchors.fill: parent
        asynchronous: true
        cache: false
        clip: true
        mipmap: true
        smooth: true
        sourceSize: Qt.size()
        source: delegate.source
    }
}
