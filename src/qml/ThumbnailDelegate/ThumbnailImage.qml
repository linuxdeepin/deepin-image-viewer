// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

Item {
    id: thumbnailImage

    property alias image: contentImage
    property url source
    property int frameIndex: 0
    property int status: imageInfo.status
    property int type

    Image {
        id: contentImage

        anchors.fill: thumbnailImage
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectCrop
        smooth: false
    }

    // 使用 ImageInfo 控制加载状态
    IV.ImageInfo {
        id: imageInfo

        frameIndex: thumbnailImage.frameIndex
        source: thumbnailImage.source

        onStatusChanged: {
            if (IV.ImageInfo.Ready === imageInfo.status
                    || IV.ImageInfo.Error === imageInfo.status) {
                if (!imageInfo.hasCachedThumbnail) {
                    contentImage.source = "qrc:/res/picture_damaged_58.svg"
                } else {
                    contentImage.source = "image://Multiimage/" + thumbnailImage.source
                            + "#frame_" + thumbnailImage.frameIndex + "_thumbnail"
                }
            }

            // 更新类型
            thumbnailImage.type = imageInfo.type
        }
    }
}
