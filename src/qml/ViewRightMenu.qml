import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
//import QtQuick.Controls 1.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Menu {
    x: 250; y: 600
    id: option_menu

    maxVisibleItems: 20

    MenuItem {
        id : right_fullscreen
        text: root.visibility != Window.FullScreen ? qsTr("Fullscreen") : qsTr("Exit fullscreen")

        onTriggered : root.visibility != Window.FullScreen ? imageViewer.showPanelFullScreen() : showNormal()
        Shortcut {
            sequence : "F11"
            onActivated : root.visibility != Window.FullScreen ? imageViewer.showPanelFullScreen() : imageViewer.escBack()
        }
        Shortcut {
            enabled: root.visibility == Window.FullScreen ? true : false
            sequence :  "Esc"
            onActivated : root.visibility != Window.FullScreen ? imageViewer.showPanelFullScreen() : imageViewer.escBack()
        }
    }


    MenuItem {
        text: qsTr("Print")
        onTriggered: {
            fileControl.showPrintDialog(mainView.source)
        }
        Shortcut {
            sequence: "Ctrl+P"
            onActivated:  {
                if (stackView.currentWidgetIndex != 0 && stackView.currentWidgetIndex != 2)
                {
                    fileControl.showPrintDialog(mainView.source)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Extract text")
        onTriggered: {
            fileControl.ocrImage(source)
        }
        Shortcut {
            sequence: "Alt+O"
            onActivated: {
                if (stackView.currentWidgetIndex != 0 && stackView.currentWidgetIndex != 2)
                {
                    fileControl.ocrImage(source)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Slide show")
        onTriggered: {startSliderShow()}
        Shortcut {
            sequence: "F5"
            onActivated: {
                if (stackView.currentWidgetIndex != 0)
                {
                    startSliderShow()
                }
            }
        }
    }


    MenuSeparator { }
    MenuItem {
        text: qsTr("Copy")
        onTriggered: {
            fileControl.copyImage(source)
        }
        Shortcut {
            sequence: "Ctrl+C"
            onActivated: {
                if (stackView.currentWidgetIndex != 0 && stackView.currentWidgetIndex != 2)
                {
                    fileControl.copyImage(source)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Rename")
        onTriggered: {
            var x = parent.mapToGlobal(0, 0).x + parent.width / 2 - 190
            var y = parent.mapToGlobal(0, 0).y + parent.height / 2 - 89
            renamedialog.setX(x)
            renamedialog.setY(y)
            renamedialog.getFileName(fileControl.slotGetFileName(source))
            renamedialog.getFileSuffix(fileControl.slotFileSuffix(source))
            renamedialog.show()
        }
        Shortcut {
            sequence: "F2"
            onActivated: {
                if (stackView.currentWidgetIndex != 0 && stackView.currentWidgetIndex != 2)
                {
                    var x = parent.mapToGlobal(0, 0).x + parent.width / 2 - 190
                    var y = parent.mapToGlobal(0, 0).y + parent.height / 2 - 89
                    renamedialog.setX(x)
                    renamedialog.setY(y)
                    renamedialog.getFileName(fileControl.slotGetFileName(source))
                    renamedialog.getFileSuffix(fileControl.slotFileSuffix(source))
                    renamedialog.show()
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Delete")
        onTriggered: {
            thumbnailListView.deleteCurrentImage()
        }
        Shortcut {
            sequence: "Delete"
            onActivated: {
                if (stackView.currentWidgetIndex == 1)
                {
                    thumbnailListView.deleteCurrentImage()
                }
            }
        }
    }

    MenuSeparator { }

    MenuItem {
        text: qsTr("Rotate clockwise")
        onTriggered: {
            imageViewer.rotateImage(90)
        }
        Shortcut {
            sequence: "Ctrl+R"
            onActivated: {
                if (stackView.currentWidgetIndex == 1)
                {
                    imageViewer.rotateImage(90)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Rotate counterclockwise")
        onTriggered: {

            imageViewer.rotateImage(-90)
        }
        Shortcut {
            sequence: "Ctrl+Shift+R"
            onActivated: {
                if (stackView.currentWidgetIndex == 1)
                {
                    imageViewer.rotateImage(-90)
                }
            }
        }
    }

    MenuItem {
        id : showNavigation

        text: !isNavShow ? qsTr("Show navigation window") : qsTr("Hide navigation window")
        onTriggered : {
            if (isNavShow) {
                isNavShow = false
                idNavWidget.visible = false
            } else {
                isNavShow = true
                idNavWidget.visible = true
                if(m_NavX === 0 && m_NavY === 0) {
                    idNavWidget.setRectPec(view.currentItem.scale) //初始蒙皮
                } else {
                    idNavWidget.setRectLocation(m_NavX, m_NavY)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Set as wallpaper")
        visible: fileControl.isSupportSetWallpaper(source)
        onTriggered: {
            fileControl.setWallpaper(source)
        }
        Shortcut {
            sequence: "Ctrl+F"
            onActivated: {
                if (stackView.currentWidgetIndex == 1)
                {
                    fileControl.setWallpaper(source)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Display in file manager")
        onTriggered: {
            fileControl.displayinFileManager(source)
        }
        Shortcut {
            sequence: "Alt+D"
            onActivated: {
                if (stackView.currentWidgetIndex == 1)
                {
                    fileControl.displayinFileManager(source)
                }
            }
        }
    }

    MenuItem {
        text: qsTr("Image info")
        onTriggered: {
            infomationDig.show()
        }
        Shortcut {
            sequence: "Ctrl+I"
            onActivated: {
                if (stackView.currentWidgetIndex == 1)
                {
                    infomationDig.show()
                }
            }
        }
    }
}
