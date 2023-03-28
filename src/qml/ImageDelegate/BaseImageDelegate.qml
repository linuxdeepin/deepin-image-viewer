// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQml 2.11
import org.deepin.image.viewer 1.0 as IV

Item {
    id: baseDelegate

    property url source
    property int type: IV.Types.NullImage
    property int status: Image.Null
    property bool isCurrentImage: index === GControl.currentIndex

    property Image targetImage
    property alias targetImageInfo: imageInfo
    property ImageInputHandler inputHandler: null

    property real rotation: 0 // 图片旋转角度
    property real scale: targetImage ? targetImage.scale : 1 // 图片缩放比例

    function reset() {
        rotation = 0
        scale = 1.0

        if (targetImage) {

            // 匹配缩放处理
            if (isCurrentImage && imageInfo.width < targetImage.width
                    && imageInfo.height < targetImage.height) {
                targetImage.scale = imageInfo.width / targetImage.paintedWidth
            } else {
                targetImage.scale = 1
            }

            targetImage.rotation = 0
        }

        // 复位图片拖拽状态，多页图将在子 Delegate 单独处理
        if (inputHandler) {
            inputHandler.reset()
        }
    }

    onIsCurrentImageChanged: {
        reset()
    }

    onStatusChanged: {
        if (Image.Ready === status || Image.Error === status) {
            reset()
        }
    }

    IV.ImageInfo {
        id: imageInfo
        source: baseDelegate.source
    }
}
