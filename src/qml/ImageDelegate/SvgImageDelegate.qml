// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV

import "../Utils"

BaseImageDelegate {
    id: delegate

    status: image.status
    targetImage: image
    inputHandler: imageInput

    Image {
        id: image

        height: delegate.height
        width: delegate.width
        asynchronous: true
        cache: false
        clip: true
        fillMode: Image.PreserveAspectFit
        mipmap: true
        smooth: true
        scale: 1.0
        source: delegate.source
        sourceSize: sourceSizeOptimizer.optimizedSourceSize

        onScaleChanged: {
            sourceSizeOptimizer.requestUpdate()
        }
    }

    SourceSizeOptimizer {
        id: sourceSizeOptimizer
        targetImage: image
        imageInfo: targetImageInfo
        delegateWidth: delegate.width
        delegateHeight: delegate.height
        maxDimension: 8000
        maxTotalPixels: 0
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
    }
}
