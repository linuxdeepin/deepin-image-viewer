// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

BaseImageDelegate {
    id: delegate

    property bool needInit: true

    status: image.status
    targetImage: image
    inputHandler: imageInput

    AnimatedImage {
        id: image

        height: delegate.height
        width: delegate.width
        asynchronous: true
        cache: false
        clip: true
        fillMode: Image.PreserveAspectFit
        smooth: true
        scale: 1.0
        source: delegate.source
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
    }

    // 动图在首次加载，状态变更为 Ready 时，paintedWidth 可能未更新，为0
    // 手动复位图片状态，调整缩放比例
    Connections {
        enabled: needInit
        target: image
        onPaintedWidthChanged: {
            if (image.paintedWidth > 0) {
                needInit = false
                delegate.reset()
            }
        }
    }
}
