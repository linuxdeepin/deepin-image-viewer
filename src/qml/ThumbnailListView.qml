// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

Item {
    id: thumbnailView

    // 用于外部获取当前缩略图栏内容的长度，用于布局, 10px为焦点缩略图不在ListView中的边框像素宽度(radius = 4 * 1.25)
    property int listContentWidth: bottomthumbnaillistView.contentWidth + 10
    // 除ListView外其它按键的占用宽度
    property int btnContentWidth: switchArrowLayout.width + leftRowLayout.width
                                  + rightRowLayout.width + deleteButton.width

//    function rotateImage(x) {
//        bottomthumbnaillistView.currentItem.rotation
//                = bottomthumbnaillistView.currentItem.rotation + x
//    }

    function deleteCurrentImage() {
        /// FIXME 移动到GControl ?
        fileControl.deleteImagePath(GControl.currentSource);
        GControl.removeImage(GControl.currentSource);
        if (0 == GControl.imageCount) {
            stackView.switchOpenImage()
        }
    }

    function previous() {
        // 切换时滑动视图不响应拖拽等触屏操作
        GStatus.viewInteractive = false

        GControl.previousImage()

//        imageViewer.recalculateLiveText()
        GStatus.viewInteractive = true
    }

    function next() {
        // 切换时滑动视图不响应拖拽等触屏操作
        GStatus.viewInteractive = false

        GControl.nextImage()

//        imageViewer.recalculateLiveText()
        GStatus.viewInteractive = true
    }

    Binding {
        delayed: true
        target: GStatus
        property: "thumbnailVaildWidth"
        value: window.width - 20 - thumbnailListView.btnContentWidth
    }

    Row {
        id: switchArrowLayout

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        spacing: 10
        leftPadding: 10

        IconButton {
            id: previousButton

            enabled: GControl.hasPreviousImage
            width: 50
            height: 50
            icon.name: "icon_previous"
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Previous")

            onClicked: {
                previous()
            }

            Shortcut {
                sequence: "Left"
                onActivated: previous()
            }
        }

        IconButton {
            id: nextButton

            enabled: GControl.hasNextImage
            width: 50
            height: 50
            icon.name: "icon_next"
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Next")

            onClicked: next()

            Shortcut {
                sequence: "Right"
                onActivated: next()
            }
        }
    }

    Row {
        id: leftRowLayout

        anchors {
            left: switchArrowLayout.right
            verticalCenter: parent.verticalCenter
        }
        spacing: 10
        leftPadding: 40
        rightPadding: 20

        IconButton {
            id: fitImageButton

            anchors.leftMargin: 30
            width: 50
            height: 50
            icon.name: "icon_11"
            enabled: !CodeImage.imageIsNull(imageViewer.source)
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Original size")

            onClicked: {
                imageViewer.fitImage()
                imageViewer.recalculateLiveText()
            }
        }

        IconButton {
            id: fitWindowButton

            width: 50
            height: 50
            icon.name: "icon_self-adaption"
            enabled: !CodeImage.imageIsNull(imageViewer.source)
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Fit to window")

            onClicked: {
                imageViewer.fitWindow()
                imageViewer.recalculateLiveText()
            }
        }

        IconButton {
            id: rotateButton

            width: 50
            height: 50
            icon.name: "icon_rotate"
            enabled: !CodeImage.imageIsNull(imageViewer.source)
                     && fileControl.isRotatable(imageViewer.source)

            onClicked: {
                imageViewer.rotateImage(-90)

                // 动态刷新导航区域图片内容，同时可在imageviewer的sourceChanged中隐藏导航区域
                // (因导航区域图片source绑定到imageviewer的source属性)
                imageViewer.source = ""
                imageViewer.source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
                imageViewer.recalculateLiveText()
            }

            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Rotate")
        }
    }

    ListView {
        id: bottomthumbnaillistView

        // 重新定位图片位置
        function rePositionView() {
            // 特殊处理，防止默认显示首个缩略图时采用Center的策略会被遮挡部分
            if (0 === currentIndex) {
                positionViewAtBeginning()
            } else {
                // 尽可能将高亮缩略图显示在列表中
                positionViewAtIndex(currentIndex, ListView.Center)
            }
        }

        anchors {
            left: leftRowLayout.right
            right: rightRowLayout.left
            verticalCenter: parent.verticalCenter
        }
        // 使用范围模式，允许高亮缩略图在preferredHighlightBegin~End的范围外，使缩略图填充空白区域
        highlightRangeMode: ListView.ApplyRange
        highlightFollowsCurrentItem: true
        height: thumbnailView.height + 10
        width: thumbnailView.width - thumbnailView.btnContentWidth
        preferredHighlightBegin: width / 2 - 25
        preferredHighlightEnd: width / 2 + 25
        clip: true
        spacing: 4
        focus: true
        orientation: Qt.Horizontal
        cacheBuffer: 200

        /// FIXME 异常的绑定
//        currentIndex: GControl.currentIndex
        Connections {
            target: GControl
            onCurrentIndexChanged: {
                bottomthumbnaillistView.currentIndex = GControl.currentIndex
            }
        }

        Connections {
            target: imageViewer

            onIsFullNormalSwitchStateChanged: {
                // 当缩放界面时，缩略图栏重新进行了布局计算，导致高亮缩略图可能不居中
                if (0 == bottomthumbnaillistView.currentIndex) {
                    bottomthumbnaillistView.positionViewAtBeginning()
                } else {
                    // 尽可能将高亮缩略图显示在列表中
                    bottomthumbnaillistView.positionViewAtIndex(bottomthumbnaillistView.currentIndex, ListView.Center)
                }
            }

            // 接收当前视图旋转角度变更信号
            onCurrentRotateChanged: {
                if (bottomthumbnaillistView.currentItem) {
                    // 计算旋转角度，限制在旋转梯度为90度，以45度为分界点
                    var rotateAngle = imageViewer.currentRotate
                    // 区分正反旋转方向ViewSection.CurrentLabelA
                    var isClockWise = rotateAngle > 0
                    // 计算绝对角度值
                    rotateAngle = Math.floor((Math.abs(rotateAngle) + 45) / 90) * 90

                    // 设置当前展示的图片的旋转方向，仅在90度方向旋转，不会跟随旋转角度(特指在触摸状态下)
                    bottomthumbnaillistView.currentItem.rotation
                            = isClockWise ? rotateAngle : -rotateAngle
                }
            }
        }

        model: GControl.globalModel
        delegate: Loader {
            id: thumbnailItemLoader

            property url source: model.imageUrl

            active: true
            asynchronous: true

            onActiveChanged: {
                if (active && imageInfo.delegateSource) {
                    setSource(imageInfo.delegateSource, { "source": thumbnailItemLoader.source  })
                }
            }

            IV.ImageInfo {
                id: imageInfo

                property url delegateSource
                property bool isCurrentItem: thumbnailItemLoader.ListView.isCurrentItem

                function checkDelegateSource() {
                    if (IV.ImageInfo.Ready !== status
                            && IV.ImageInfo.Error !== status) {
                        return
                    }

                    if (IV.Types.MultiImage === type && isCurrentItem) {
                        delegateSource = "qrc:/qml/ThumbnailDelegate/MultiThumnailDelegate.qml"
                    } else {
                        delegateSource = "qrc:/qml/ThumbnailDelegate/NormalThumbnailDelegate.qml"
                    }
                }

                source: thumbnailItemLoader.source

                onDelegateSourceChanged: {
                    if (thumbnailItemLoader.active && delegateSource) {
                        setSource(delegateSource, { "source": thumbnailItemLoader.source })
                    }
                }

                onStatusChanged: {
                    checkDelegateSource()
                }

                onIsCurrentItemChanged: {
                    checkDelegateSource()
                }
            }
        }

        //滑动联动主视图
        onCurrentIndexChanged: {
            if (currentItem) {
                currentItem.forceActiveFocus()
            }

            rePositionView()

            // 仅在边缘缩略图时进行二次定位
            if (0 === currentIndex || currentIndex === (count - 1)) {
                delayUpdateTimer.restart()
            }
        }

        Timer {
            id: delayUpdateTimer

            repeat: false
            interval: 100
            onTriggered: {
                bottomthumbnaillistView.forceLayout()
                bottomthumbnaillistView.rePositionView()
            }
        }

        // 添加两组空的表头表尾用于占位，防止在边界的高亮缩略图被遮挡, 5px为不在ListView中维护的焦点缩略图边框的宽度 radius = 4 * 1.25
        header: Rectangle {
            width: 5
        }

        footer: Rectangle {
            width: 5
        }

        Behavior on x {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }

        Behavior on y {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }

        Component.onCompleted: {
            bottomthumbnaillistView.currentIndex = GControl.currentIndex
        }
    }

    Row {
        id: rightRowLayout

        anchors {
            right: deleteButton.left
            verticalCenter: parent.verticalCenter
        }
        spacing: 10
        leftPadding: 20
        rightPadding: 20

        IconButton {
            id: ocrButton

            width: 50
            height: 50
            enabled: fileControl.isCanSupportOcr(source)
                     && !CodeImage.imageIsNull(source)
            icon.name: "icon_character_recognition"
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Extract text")

            onClicked: fileControl.ocrImage(source, imageViewer.frameIndex)
        }
    }

    IconButton {
        id: deleteButton

        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
        }
        enabled: fileControl.isCanDelete(GControl.currentSource)
        width: 50
        height: 50
        icon.name: "icon_delete"
        icon.source: "qrc:/res/dcc_delete_36px.svg"
        icon.color: enabled ? "red" : "ffffff"
        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Delete")

        onClicked: deleteCurrentImage()
    }
}
