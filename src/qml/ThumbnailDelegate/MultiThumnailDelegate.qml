// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.image.viewer 1.0 as IV

// 用于多页图的缩略图代理
BaseThumbnailDelegate {
    id: multiThumnailDelegate

    height: 50
    width: {
        // 最少需要保留两张图片显示的大小
        var minWidth = 30 + 11
        // 计算允许的多页图显示宽度，宽度计算以当前界面窗口的宽度计算，此处取宽度值
        var enableWidth = GStatus.thumbnailVaildWidth - (30 * 2) - 20
        enableWidth = Math.max(enableWidth, minWidth)

        // 每张子图片最多占用30px，间隔1px
        var curMultiImageWidth = (31 * listView.count) - 1
        return Math.min(619, Math.min(curMultiImageWidth, enableWidth))
    }
    shader.visible: true
    shader.z: 1
    y: 15
    imgRadius: 4

    ListView {
        id: listView

        // 计算调整的 item 宽度，item 宽度允许范围内处于 10px ~ 30px, count一定 >=2
        property int preferredItemWidth: Math.min(Math.max(10, (width - 30) / (count - 1)), 30)

        anchors.fill: parent
        orientation: Qt.Horizontal
        cacheBuffer: 200
        spacing: 1
        focus: true
        // 使用范围模式，允许高亮缩略图在preferredHighlightBegin~End的范围外，使缩略图填充空白区域
        highlightRangeMode: ListView.ApplyRange
        highlightFollowsCurrentItem: true
        // 使焦点图片尽量显示在缩略图栏中部
        preferredHighlightBegin: width / 2 - 25
        preferredHighlightEnd: width / 2 + 25
        // 上层存在另一 ListView 底部缩略图栏，此 ListView 设置拖动不可超过边界，不影响二者的拖拽效果
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        currentIndex: GControl.currentFrameIndex

        model: imageInfo.frameCount
        delegate: Item {
            id: delegateItem

            height: listView.height
            width: listView.preferredItemWidth

            ThumbnailImage {
                id: img

                anchors.fill: parent
                frameIndex: index
                source: multiThumnailDelegate.source
            }

            // 焦点图片边框
            Rectangle {
                id: imgBorder

                anchors.fill: parent
                color: parent.ListView.isCurrentItem ? "transparent" : Qt.rgba(0, 0, 0, 0.3)
                border.width: 0
                border.color: "white"
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    // 向外发送多页图帧号切换事件
                    GControl.currentFrameIndex = index;
                }
            }

            // 焦点状态的图片单独展示
            states: State {
                name: "active"
                when: ListView.isCurrentItem

                PropertyChanges {
                    target: delegateItem
                    width: 30
                }

                PropertyChanges {
                    target: imgBorder
                    border.width: 1
                }
            }
        }

        // 滑动联动列表
        onCurrentIndexChanged: {
            // 特殊处理，防止默认显示首个缩略图时采用Center的策略会被遮挡部分
            if (0 == currentIndex) {
                positionViewAtBeginning()
            } else {
                // 尽可能将高亮缩略图显示在列表中
                positionViewAtIndex(currentIndex, ListView.Center)
            }
        }
    }

    // 进入代理时，图片信息一定加载完成
    IV.ImageInfo {
        id: imageInfo
        source: multiThumnailDelegate.source
    }
}
