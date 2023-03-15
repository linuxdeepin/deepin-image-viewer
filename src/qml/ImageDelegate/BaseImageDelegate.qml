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

    property var targetImage
    property real sourceWidth: targetImage ? targetImage.sourceSize.width : -1 // 图片源宽度
    property real sourceHeight: targetImage ? targetImage.sourceSize.height : -1 // 图片源高度
    property real paintedWidth: targetImage ? targetImage.paintedWidth : -1
    property real paintedHeight: targetImage ? targetImage.paintedHeight: -1
    property real rotation: 0 // 图片旋转角度
    property real scale: targetImage ? targetImage.scale : 1 // 图片缩放比例

    function reset() {
        rotation = 0
        scale = 1.0

        if (undefined !== targetImage) {
            targetImage.scale = 1.0
            targetImage.rotation = 0
        }
    }

    function scaleUp() {

    }

    function scaleDown() {

    }

    //! \test 调试使用
    onTargetImageChanged: {
        if (undefined !== targetImage) {
            console.warn("------------------targetImage:", targetImage.source, sourceWidth, sourceHeight)
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
