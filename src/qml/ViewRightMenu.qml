
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
//import QtQuick.Controls 1.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

Menu {
    id: option_menu

    property url imageSource: GControl.currentSource
    property bool isImageShowPage: stackView.stackPage === Number(IV.Types.OpenImagePage)

    x: 250
    y: 600
    maxVisibleItems: 20

    RightMenuItem {
        id: right_fullscreen

        text: root.visibility != Window.FullScreen ? qsTr("Fullscreen") : qsTr("Exit fullscreen")
        onTriggered: showFulltimer.start()

        Shortcut {
            sequence: "F11"
            onActivated: root.visibility
                         != Window.FullScreen ? imageViewer.showPanelFullScreen() : imageViewer.escBack()
        }

        Shortcut {
            enabled: root.visibility == Window.FullScreen ? true : false
            sequence: "Esc"
            onActivated: root.visibility
                         != Window.FullScreen ? imageViewer.showPanelFullScreen() : imageViewer.escBack()
        }
    }

    RightMenuItem {
        text: qsTr("Print")

        visible: !CodeImage.imageIsNull(imageSource)

        onTriggered: {
            fileControl.showPrintDialog(imageSource)
        }

        Shortcut {
            sequence: "Ctrl+P"
            enabled: !CodeImage.imageIsNull(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    fileControl.showPrintDialog(imageSource)
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Extract text")
        visible: fileControl.isCanSupportOcr(imageSource) && !CodeImage.imageIsNull(imageSource)
        onTriggered: {
            fileControl.ocrImage(imageSource, GControl.currentFrameIndex)
        }
        Shortcut {
            sequence: "Alt+O"
            enabled: fileControl.isCanSupportOcr(imageSource)
                     && !CodeImage.imageIsNull(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    fileControl.ocrImage(imageSource, GControl.currentFrameIndex)
                }
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
                if (parent.visible && Number(IV.Types.ImageViewPage) === stackView.stackPage) {
                    stackView.switchSliderShow()
                }
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
            if (parent.visible) {
                fileControl.copyImage(imageSource)
            }
        }
        Shortcut {
            sequence: "Ctrl+C"
            enabled: fileControl.isCanReadable(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    fileControl.copyImage(imageSource)
                }
            }
        }
    }

    RightMenuItem {

        text: qsTr("Rename")
        visible: fileControl.isCanRename(imageSource)
        onTriggered: {
            var x = parent.mapToGlobal(0, 0).x + parent.width / 2 - 190
            var y = parent.mapToGlobal(0, 0).y + parent.height / 2 - 89
            renamedialog.setX(x)
            renamedialog.setY(y)
            renamedialog.getFileName(fileControl.slotGetFileName(imageSource))
            renamedialog.getFileSuffix(fileControl.slotFileSuffix(imageSource))
            renamedialog.show()
        }
        Shortcut {
            sequence: "F2"
            // 判断文件是否允许重命名
            enabled: fileControl.isCanRename(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    var x = parent.mapToGlobal(0, 0).x + parent.width / 2 - 190
                    var y = parent.mapToGlobal(0, 0).y + parent.height / 2 - 89
                    renamedialog.setX(x)
                    renamedialog.setY(y)
                    renamedialog.getFileName(fileControl.slotGetFileName(imageSource))
                    renamedialog.getFileSuffix(fileControl.slotFileSuffix(imageSource))
                    renamedialog.show()
                }
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
            enabled: fileControl.isCanDelete(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    thumbnailListView.deleteCurrentImage()
                }
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
        visible: !CodeImage.imageIsNull(imageSource)
                 && fileControl.isRotatable(imageSource)
        onTriggered: {
            imageViewer.rotateImage(90)
        }

        Shortcut {
            sequence: "Ctrl+R"
            enabled: !CodeImage.imageIsNull(imageSource) && fileControl.isRotatable(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    imageViewer.rotateImage(90)
                }
            }
        }
    }

    RightMenuItem {
        text: qsTr("Rotate counterclockwise")
        visible: !CodeImage.imageIsNull(imageSource)
                 && fileControl.isRotatable(imageSource)
        onTriggered: {
            imageViewer.rotateImage(-90)
        }
        Shortcut {
            sequence: "Ctrl+Shift+R"
            enabled: !CodeImage.imageIsNull(imageSource)
                     && fileControl.isRotatable(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    imageViewer.rotateImage(-90)
                }
            }
        }
    }

    RightMenuItem {
        id: showNavigation

        visible: !CodeImage.imageIsNull(imageSource) && currentScale > 1
                 && root.height > global.minHideHeight
                 && root.width > global.minWidth
        text: !imageViewer.isNavShow ? qsTr("Show navigation window") : qsTr(
                                           "Hide navigation window")

        onTriggered: {
            if (!parent.visible) {
                return
            }

            if (imageViewer.isNavShow) {
                imageViewer.isNavShow = false
                idNavWidget.visible = false
            } else {
                imageViewer.isNavShow = true
                idNavWidget.visible = true
                if (m_NavX === 0 && m_NavY === 0) {
                    // 设置蒙皮信息
                    idNavWidget.setRectPec(view.currentItem.scale,
                                           imageViewer.viewImageWidthRatio,
                                           imageViewer.viewImageHeightRatio)
                } else {
                    idNavWidget.setRectLocation(m_NavX, m_NavY)
                }
            }
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
            enabled: fileControl.isSupportSetWallpaper(imageSource)
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    fileControl.setWallpaper(imageSource)
                }
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
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    fileControl.displayinFileManager(imageSource)
                }
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
            onActivated: {
                if (parent.visible && stackView.currentWidgetIndex == 1) {
                    infomationDig.show()
                }
            }
        }
    }
}
