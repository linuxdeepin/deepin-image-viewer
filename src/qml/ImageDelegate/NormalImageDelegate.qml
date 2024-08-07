// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV
import "../Utils"

BaseImageDelegate {
    id: delegate

    inputHandler: imageInput
    status: image.status
    targetImage: image

    Image {
        id: image

        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectFit
        height: delegate.height
        mipmap: true
        scale: 1.0
        smooth: true
        source: "image://ImageLoad/" + delegate.source + "#frame_" + delegate.frameIndex
        width: delegate.width

        Behavior on rotation {
            NumberAnimation {
                duration: 366
                easing.type: Easing.InOutQuad
            }
        }
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        isRotatable: IV.FileControl.isRotatable(delegate.source)
        targetImage: image.status === Image.Ready ? image : null
    }

    Connections {
        function onCurrentRotationChanged() {
            // Note: 确保缓存中的数据已刷新后更新界面
            // 0 为复位，缓存中的数据已转换，无需再次加载
            if (0 !== IV.GControl.currentRotation) {
                var temp = image.source;
                image.source = "";
                image.source = temp;
            }
        }

        enabled: isCurrentImage
        target: IV.GControl
    }
}
