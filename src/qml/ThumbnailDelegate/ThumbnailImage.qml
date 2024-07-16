// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

Item {
    id: thumbnailImage

    property alias frameCount: imageInfo.frameCount
    property int frameIndex: 0
    property alias image: contentImage
    property url source
    property int status: imageInfo.status
    property int type

    function reset() {
        var temp = contentImage.source;
        contentImage.source = "";
        contentImage.source = temp;
    }

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
            var imageSource = "image://ThumbnailLoad/" + thumbnailImage.source + "#frame_" + thumbnailImage.frameIndex;
            if (IV.ImageInfo.Error === imageInfo.status) {
                if (imageInfo.hasCachedThumbnail) {
                    contentImage.source = imageSource;
                } else {
                    contentImage.source = "qrc:/res/picture_damaged_58.svg";
                }
            } else if (IV.ImageInfo.Ready === imageInfo.status) {
                contentImage.source = imageSource;
            }

            // 更新类型，不直接绑定
            thumbnailImage.type = imageInfo.type;
        }
    }
}
