// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

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
        smooth: true
        mipmap: true
        fillMode: Image.PreserveAspectFit
        scale: 1.0
        source: "image://ImageLoad/" + delegate.source
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
        isRotatable: fileControl.isRotatable(delegate.source)
    }

    Connections {
        enabled: isCurrentImage
        target: GControl
        onCurrentRotationChanged: {
            // Note: 确保缓存中的数据已刷新后更新界面
            // 0 为复位，缓存中的数据已转换，无需再次加载
            if (0 !== GControl.currentRotation) {
                var temp = image.source
                image.source = ""
                image.source = temp
            }
        }
    }
}
