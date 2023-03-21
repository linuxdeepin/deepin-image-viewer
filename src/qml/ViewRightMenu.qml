// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

Menu {
    id: optionMenu

    property url imageSource: GControl.currentSource
    property bool isNullImage: imageInfo.type === IV.Types.NullImage

    x: 250
    y: 600
    maxVisibleItems: 20

    RightMenuItem {
        id: rightFullscreen

        text: !window.isFullScreen ? qsTr("Fullscreen") : qsTr("Exit fullscreen")
        onTriggered: showFulltimer.start()

        Shortcut {
            sequence: "F11"
            onActivated: !window.isFullScreen ? imageViewer.showPanelFullScreen() : imageViewer.escBack()
        }

        Shortcut {
            enabled: window.isFullScreen
            sequence: "Esc"
            onActivated: !window.isFullScreen ? imageViewer.showPanelFullScreen() : imageViewer.escBack()
        }
    }

    RightMenuItem {
        text: qsTr("Print")
        visible: !isNullImage

        onTriggered: {
            optionMenu.close()
            fileControl.showPrintDialog(imageSource)
        }

        Shortcut {
            sequence: "Ctrl+P"

            onActivated: {
                optionMenu.close()
                fileControl.showPrintDialog(imageSource)
            }
        }
    }

    RightMenuItem {
        text: qsTr("Extract text")
        visible: fileControl.isCanSupportOcr(imageSource) && !isNullImage

        onTriggered: {
            fileControl.ocrImage(imageSource, GControl.currentFrameIndex)
        }

        Shortcut {
            sequence: "Alt+O"
            enabled: parent.visible
            onActivated: {
                fileControl.ocrImage(imageSource, GControl.currentFrameIndex)
            }
        }
    }

    RightMenuItem {
        text: qsTr("Slide show")

        onTriggered: {
            stackView.switchSliderShow()
        }

        Shortcut {
            sequence: "F5"
            onActivated: {
                stackView.switchSliderShow()
            }
        }
    }

    MenuSeparator {
        id: firstSeparator
    }

    RightMenuItem {
        text: qsTr("Copy")
        visible: fileControl.isCanReadable(imageSource)

        onTriggered: {
            fileControl.copyImage(imageSource)
        }

        Shortcut {
            sequence: "Ctrl+C"
            enabled: parent.visible
            onActivated: {
                fileControl.copyImage(imageSource)
            }
        }
    }

    RightMenuItem {
        text: qsTr("Rename")
        visible: fileControl.isCanRename(imageSource)

        onTriggered: {
            renamedialog.show()
        }

        Shortcut {
            sequence: "F2"
            // 判断文件是否允许重命名
            enabled: parent.visible
            onActivated: {
                renamedialog.show()
            }
        }
    }

    RightMenuItem {
        text: qsTr("Delete")
        visible: fileControl.isCanDelete(imageSource)

        onTriggered: {
            thumbnailListView.deleteCurrentImage()
        }

        Shortcut {
            sequence: "Delete"
            enabled: parent.visible
            onActivated: {
                thumbnailListView.deleteCurrentImage()
            }
        }
    }

    // 不允许无读写权限时上方选项已屏蔽，不展示此分割条
    MenuSeparator {
        // 不显示分割条时调整高度，防止菜单项间距不齐
        height: visible ? firstSeparator.height : 0
        visible: fileControl.isCanReadable(imageSource) || fileControl.isCanDelete(imageSource)
    }

    RightMenuItem {
        text: qsTr("Rotate clockwise")
        visible: !isNullImage && fileControl.isRotatable(imageSource)

        onTriggered: {
            imageViewer.rotateImage(90)
        }

        Shortcut {
            sequence: "Ctrl+R"
            enabled: parent.visible
            onActivated: {
                imageViewer.rotateImage(90)
            }
        }
    }

    RightMenuItem {
        text: qsTr("Rotate counterclockwise")
        visible: !isNullImage && fileControl.isRotatable(imageSource)

        onTriggered: {
            imageViewer.rotateImage(-90)
        }

        Shortcut {
            sequence: "Ctrl+Shift+R"
            enabled: parent.visible
            onActivated: {
                imageViewer.rotateImage(-90)
            }
        }
    }

    RightMenuItem {
        id: showNavigation

        visible: !isNullImage
                 && window.height > GStatus.minHideHeight
                 && window.width > GStatus.minWidth
        text: !GStatus.showNavigation ? qsTr("Show navigation window") : qsTr("Hide navigation window")

        onTriggered: {
            if (!parent.visible) {
                return
            }

            GStatus.showNavigation = !GStatus.showNavigation
        }
    }

    RightMenuItem {
        text: qsTr("Set as wallpaper")
        visible: fileControl.isSupportSetWallpaper(imageSource)

        onTriggered: {
            fileControl.setWallpaper(imageSource)
        }

        Shortcut {
            sequence: "Ctrl+F9"
            enabled: parent.visible
            onActivated: {
                fileControl.setWallpaper(imageSource)
            }
        }
    }

    RightMenuItem {
        text: qsTr("Display in file manager")

        onTriggered: {
            fileControl.displayinFileManager(imageSource)
        }

        Shortcut {
            sequence: "Alt+D"
            enabled: parent.visible
            onActivated: {
                fileControl.displayinFileManager(imageSource)
            }
        }
    }

    RightMenuItem {
        text: qsTr("Image info")

        onTriggered: {
            infomationDig.show()
        }

        Shortcut {
            sequence: "Ctrl+I"
            enabled: parent.visible
            onActivated: {
                infomationDig.show()
            }
        }
    }

    IV.ImageInfo {
        id: imageInfo
        source: imageSource

        onStatusChanged: {
            if (IV.ImageInfo.Ready === imageInfo.status) {
                isNullImage = (imageInfo.type === IV.Types.NullImage)
            } else {
                isNullImage = true
            }
        }
    }
}
