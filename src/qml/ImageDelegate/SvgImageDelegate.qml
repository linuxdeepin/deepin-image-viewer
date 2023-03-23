// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

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
        scale: delegate.scale
        source: delegate.source
        // SVG 图片源大小需单独设置
        sourceSize.width: imageInfo.width
        sourceSize.height: imageInfo.height
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
    }

    IV.ImageInfo {
        id: imageInfo
        source: delegate.source
    }
}
