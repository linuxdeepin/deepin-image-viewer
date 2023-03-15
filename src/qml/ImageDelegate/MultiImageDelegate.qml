// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

// 使用嵌套的ListView进行浏览
BaseImageDelegate {
    id: multiImageDelegate

    ListView {
        id: multiImageView

        width: multiImageDelegate.width
        height: multiImageDelegate.height
        cacheBuffer: 200
        clip: true
        boundsMovement: Flickable.FollowBoundsBehavior
        boundsBehavior: Flickable.StopAtBounds
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightMoveDuration: 0
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        flickDeceleration: 500
        currentIndex: {
            if (isCurrentImage) {
                return GControl.currentFrameIndex
            } else if (index === GControl.currentIndex + 1) {
                return 0
            } else if (index === GControl.currentIndex - 1) {
                return count - 1
            }

            return 0
        }

        // 当处理双击缩放界面时，由于坐标变更，可能误触导致图片滑动
        // 调整为在缩放动作时不处理滑动操作
        //        interactive: !imageViewer.isFullNormalSwitchState
        //                     && imageViewer.viewInteractive
        model: imageInfo.frameCount
        delegate: Loader {
            width: multiImageDelegate.width
            height: multiImageDelegate.height
            active: {
                if (ListView.isCurrentItem) {
                    return true
                }
                if (multiImageView.currentIndex - 1 === index
                        || multiImageView.currentIndex + 1 === index) {
                    return true
                }
                return false
            }
            asynchronous: multiImageView.currentIndex - 1 === index
                          || multiImageView.currentIndex + 1 === index

            sourceComponent: Image {
                id: image

                property bool isCurrentImage: parent.ListView.isCurrentItem

                // TIF 图片暂无旋转功能
                height: parent.height
                width: parent.width
                asynchronous: true
                cache: false
                fillMode: Image.PreserveAspectFit
                mipmap: true
                smooth: true
                scale: isCurrentImage ? multiImageDelegate.scale : 1.0
                source: "image://Multiimage/" + multiImageDelegate.source + "#frame_" + index

                onIsCurrentImageChanged: {
                    if (isCurrentImage) {
                        multiImageDelegate.targetImage = this
                    }

                    multiImageDelegate.reset()
                    imageInput.reset()
                }

                Binding {
                    target: multiImageDelegate
                    property: "status"
                    value: image.status
                    when: isCurrentImage
                }

                ImageInputHandler {
                    id: imageInput

                    anchors.fill: parent
                    targetImage: image

                    onVisibleChanged: {
                        console.warn("--------------------visible change", visible)
                    }
                }
            }
        }

        onCurrentIndexChanged: {
            if (isCurrentImage && currentIndex != GControl.currentFrameIndex) {
                GControl.currentFrameIndex = currentIndex
            }
        }

        onMovementStarted: {
            GStatus.viewFlicking = true
        }

        onMovementEnded: {
            GStatus.viewFlicking = false
        }
    }

    IV.ImageInfo {
        id: imageInfo
        source: multiImageDelegate.source
    }
}
