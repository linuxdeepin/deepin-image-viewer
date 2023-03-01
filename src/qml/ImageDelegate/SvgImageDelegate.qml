// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

BaseImageDelegate {
    id: delegate

    status: image.status
    targetImage: image

    Image {
        id: image

        height: delegate.height
        width: delegate.width
        asynchronous: true
        cache: false
        clip: true
        mipmap: true
        smooth: true
        sourceSize: Qt.size()
        scale: delegate.scale
        source: delegate.source
    }
}
