// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

BaseThumbnailDelegate {
    id: normalThumbnailDelegate

    // 判断是否为多页图
    property bool isMultiImage: IV.Types.MultiImage === img.type

    Item {
        anchors.fill: parent
        visible: true

        ThumbnailImage {
            id: img
            anchors.fill: parent
            source: normalThumbnailDelegate.source
        }

        Rectangle {
            id: maskRect
            anchors.fill: img
            visible: false
            radius: imgRadius
        }

        OpacityMask {
            id: imgMask
            anchors.fill: img
            source: img
            maskSource: maskRect
        }
    }

    // 图片数角标
    Loader {
        id: anchorLoader

        height: 14
        width: Math.max(20, implicitWidth)
        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        // 非多页图无需实例化
        active: isMultiImage
        // 仅多页图显示角标(为焦点时不加载)
        visible: isMultiImage

        sourceComponent: Rectangle {
            id: anchorRect

            anchors.fill: parent
            radius: 4

            // 多页图图片数角标
            DTK.Label {
                id: anchorLabel

                anchors.fill: parent
                topPadding: 3
                bottomPadding: 3
                leftPadding: 2
                rightPadding: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.DemiBold
                font.pixelSize: 11
                text: {
                    // 取得当前索引的图片帧号
                    var count = fileControl.getImageCount(normalThumbnailDelegate.source)
                    return (count <= 999) ? count : "999+"
                }

                background: Rectangle {
                    implicitHeight: 14
                    implicitWidth: 14
                    radius: 4

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: "#FFC3C3C3"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#FFD8D8D8"
                        }
                    }
                }
            }

            // 图片角标的内阴影
            InnerShadow {
                anchors.fill: anchorLabel
                verticalOffset: -1
                color: Qt.rgba(0, 0, 0, 0.1)
                source: anchorLabel
            }

            // 图片角标的外阴影
            DropShadow {
                anchors.fill: anchorLabel
                verticalOffset: 1
                cached: true
                radius: 2
                samples: 4
                color: Qt.rgba(0, 0, 0, 0.3)
                source: anchorLabel
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            GControl.currentIndex = index
        }
    }

    states: [
        // 激活状态
        State {
            name: "active"
            when: isCurrentItem && !isMultiImage

            PropertyChanges {
                target: normalThumbnailDelegate
                y: 15
                height: 50
                width: height
                imgRadius: 4
            }

            PropertyChanges {
                target: normalThumbnailDelegate.shader
                visible: true
                z: 1
            }
        }
    ]
}
