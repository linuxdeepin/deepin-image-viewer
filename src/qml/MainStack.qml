// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.3
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

Item {
    id: stackView

    anchors.fill: parent

    // 打开图片对话框
    function openImageDialog() {
        if (Loader.Ready === fileDialogLoader.status) {
            fileDialogLoader.item.open()
        } else {
            fileDialogLoader.active = true
        }
    }

    // 设置当前使用的图片源
    function setSourcePath(path) {
        var sourcePaths = fileControl.getDirImagePath(path)
        if (sourcePaths.length > 0) {
            GControl.setImageFiles(sourcePaths, path)
            // 记录当前读取的图片信息
            fileControl.resetImageFiles(sourcePaths)

            console.log("Load image info", path)

            switchImageView()
        } else {
            switchOpenImage()
        }
    }

    function switchOpenImage() {
        GStatus.stackPage = Number(IV.Types.OpenImagePage)
        window.title = ""
        contentLoader.setSource("qrc:/qml/OpenImageWidget.qml");
    }

    function switchImageView() {
        GStatus.stackPage = Number(IV.Types.ImageViewPage)
        contentLoader.setSource("qrc:/qml/FullImageView.qml");
    }

    function switchSliderShow() {
        if (Number(IV.Types.ImageViewPage) === stackPage) {
            GStatus.stackPage = Number(IV.Types.SliderShowPage)
            contentLoader.setSource("qrc:/qml/SliderShow.qml");
        }
    }

    Control {
        id: backcontrol

        property DTK.Palette backgroundColor: DTK.Palette {
            normal: "#F8F8F8"
            normalDark: "#000000"
        }

        hoverEnabled: true // 开启 Hover 属性
    }

    Connections {
        target: fileControl

        // 关联外部通过 DBus 等方式触发调用看图
        onOpenImageFile: {
            setSourcePath(fileName)
        }
    }

    // 标题栏
    ViewTopTitle {
        id: titleRect
        z: parent.z + 1
    }

    // 展示内容
    Loader {
        id: contentLoader

        active: true
        anchors.fill: parent
    }

    DropArea {
        id: dropArea

        anchors.fill: parent

        onEntered: {
            background.color = "gray"
            drag.accept(Qt.CopyAction)
        }

        onDropped: {
            if (drop.hasUrls && drop.urls.length !== 0) {
                setSourcePath(drop.urls[0])
            }
        }

        onExited: {
            background.color = "white"
            console.log("onExited")
        }
    }

    Loader {
        id: fileDialogLoader

        active: false
        asynchronous: true
        sourceComponent: FileDialog {
            id: fileDialog

            title: qsTr("Select pictures")
            folder: shortcuts.pictures
            selectMultiple: true
            nameFilters: ["Image files (*.jpg *.png *.bmp *.gif *.ico *.jpe "
                + "*.jps *.jpeg *.jng *.koala *.koa *.lbm "
                + "*.iff *.mng *.pbm *.pbmraw *.pcd *.pcx "
                + "*.pgm *.pgmraw *.ppm *.ppmraw *.ras *.tga "
                + "*.targa *.tiff *.tif *.wbmp *.psd *.cut *.xbm "
                + "*.xpm *.dds *.fax *.g3 *.sgi *.exr *.pct *.pic "
                + "*.pict *.webp *.jxr *.mrw *.raf *.mef *.raw *.orf "
                + "*.djvu *.or2 *.icns *.dng *.svg *.nef *.pef *.pxm *.pnm)"]

            onAccepted: {
                stackView.setSourcePath(fileDialog.fileUrls[0])
            }

            Component.onCompleted: {
                fileDialog.open()
            }
        }
    }

    // 快捷键打开帮助手册
    Shortcut {
        enabled: true
        autoRepeat: false
        sequence: "F1"
        onActivated: {
            D.ApplicationHelper.handleHelpAction()
        }
    }

    // 打开图片文件
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: {
            // 不在动画展示状态
            if (Number(IV.Types.SliderShowPage) !== GStatus.stackPage) {
                openImageDialog()
            }
        }
    }

    Component.onCompleted: {
        // 从命令行启动时取得命令行参数
        stackView.setSourcePath(fileControl.parseCommandlineGetPath("x"))
    }
}
