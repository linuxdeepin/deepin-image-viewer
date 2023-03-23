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
    property real paintedWidth: targetImage ? targetImage.paintedWidth : -1
    property real paintedHeight: targetImage ? targetImage.paintedHeight: -1
    property real rotation: 0 // 图片旋转角度
    property real scale: targetImage ? targetImage.scale : 1 // 图片缩放比例

    function reset() {
        rotation = 0
        scale = 1.0

        if (targetImage) {
            // 匹配缩放处理
            if (isCurrentImage && targetImage.sourceSize.height < targetImage.height) {
                targetImage.scale = targetImage.sourceSize.width / targetImage.paintedWidth
            } else {
                targetImage.scale = 1
            }

            targetImage.rotation = 0
        }
    }

    onIsCurrentImageChanged: {
        reset()
    }

    onStatusChanged: {
        if (Image.Ready === status
                || Image.Error === status) {
            reset()
        }
    }
}
