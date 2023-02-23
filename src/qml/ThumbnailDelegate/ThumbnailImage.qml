// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
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
            if (IV.ImageInfo.Ready === imageInfo.status) {
                if (IV.Types.DamagedImage === imageInfo.type) {
                    contentImage.source = "qrc:/res/picture_damaged_58.svg"
                } else {
                    contentImage.source = "image://multiimage/" + thumbnailImage.source
                            + "#frame_" + thumbnailImage.frameIndex + "_thumbnail"
                }
            } else if (IV.ImageInfo.Error === imageInfo.status) {
                contentImage.source = "qrc:/res/picture_damaged_58.svg"
            }

            // 更新类型
            thumbnailImage.type = imageInfo.type
        }

        onInfoChanged: {
            // 变更前已为加载状态，则是信息更新，需重新加载
            if (contentImage.source
                    && IV.ImageInfo.Ready === imageInfo.status) {
                /// FIXME 判断初始化是否被重复加载了
                var old = contentImage.source
                contentImage.source = undefine
                contentImage.source = old
            }
        }
    }
}
