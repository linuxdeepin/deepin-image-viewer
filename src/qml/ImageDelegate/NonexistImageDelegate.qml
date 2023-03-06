// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

BaseImageDelegate {
    id: delegate

    status: notExistImage.status

    Image {
        id: notExistImage

        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        width: 100
        height: 100
        smooth: true
        mipmap: true
        source: imageInfo.hasCachedThumbnail
                ? "image://Multiimage/" + delegate.source
                : "qrc:/res/icon_import_photo.svg"
    }

    Text {
        // 提示图片未找到信息
        id: notExistLabel

        anchors {
            top: notExistImage.bottom
            topMargin: 20
            horizontalCenter: notExistImage.horizontalCenter
        }
        text: qsTr("Image file not found")
        textFormat: Text.PlainText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    IV.ImageInfo {
        id: imageInfo
        source: delegate.source
    }
}
