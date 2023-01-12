// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

Item {

    property string imageSource
    property string imageSourceTemp

    function restart() {
        destroyAnimation.complete()
        createAnimation.start()
    }

    onImageSourceChanged:
    {
        destroyAnimation.start()
        createAnimation.start()
    }

    Image {
        id: toBeDeleted
        fillMode:Image.PreserveAspectFit
        width: parent.width
        height: parent.height
//        sourceSize.width: parent.width
//        sourceSize.height: parent.width
//        anchors.centerIn: parent
        anchors.fill: parent

        source: imageSourceTemp
        NumberAnimation on opacity {
            id: destroyAnimation
            from: 1
            to: 0
            duration: 2000
            easing.type: Easing.InOutQuad

            onRunningChanged: {
             if (!running) {

             }
           }
        }
    }

    Image {
        id: toBeCreated
        fillMode:Image.PreserveAspectFit
        width:parent.width
        height:parent.width
//        fillMode:Image.PreserveAspectFit
//        sourceSize.width: parent.width
//        sourceSize.height: parent.width
        anchors.centerIn: parent
        anchors.fill: parent
        source: imageSource
        NumberAnimation on opacity {
            id: createAnimation
            from: 0
            to: 1
            duration: 2000
            easing.type: Easing.InOutQuad

            onRunningChanged: {
                 if (!running) {
                    imageSourceTemp = imageSource
                 }
            }
        }
    }
}
