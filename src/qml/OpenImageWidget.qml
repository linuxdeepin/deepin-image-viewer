// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.3
import Qt.labs.folderlistmodel 2.11
import org.deepin.dtk 1.0


Item {
    id: openwidget

    property string file
    // 弹出对话框对象，用于外部调用打开对话框
    property alias openFileDialog: fileDialog

    function openImageFile(fileName) {
        mainView.sourcePaths = fileControl.getDirImagePath(fileName);
        mainView.source = fileName

        if (mainView.sourcePaths.length > 0) {
            // 记录当前读取的图片信息，用于监控文件变更
            fileControl.resetImageFiles(mainView.sourcePaths)

            mainView.setThumbnailCurrentIndex(mainView.sourcePaths.indexOf(mainView.source))
            stackView.currentWidgetIndex = 1

            console.log("Open image file", fileName)
        }
    }

    Connections {
        target: fileControl

        // 关联外部通过 DBus 等方式触发调用看图
        onOpenImageFile: {
            openwidget.openImageFile(fileName)
        }
    }

    Rectangle{
        id:openRec
        color:backcontrol.ColorSelector.backgroundColor
        anchors.fill: parent
        anchors.centerIn: openwidget

        ActionButton {
            id: openWidgetImage
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            icon {
                name:"import_photo"
                width: 128
                height: 128
            }
        }

        RecommandButton {
            id: openFileBtn
            font.capitalization: Font.MixedCase
            text: qsTr("Open Image")

            onClicked: fileDialog.open()
            width: 300
            height: 35
            anchors.top:openWidgetImage.bottom
            anchors.topMargin:10

            anchors.left : openWidgetImage.left
            anchors.leftMargin: -86

        }
    }

    Shortcut {
        sequence: "Ctrl+O"
        onActivated:{
            if(stackView.currentWidgetIndex!= 2){
                fileDialog.open()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Select pictures")
        folder: shortcuts.pictures
        selectMultiple: true

        nameFilters: ["Image files (*.jpg *.png *.bmp *.gif *.ico *.jpe *.jps *.jpeg *.jng *.koala *.koa *.lbm *.iff *.mng *.pbm *.pbmraw *.pcd *.pcx *.pgm *.pgmraw *.ppm *.ppmraw *.ras *.tga *.targa *.tiff *.tif *.wbmp *.psd *.cut *.xbm *.xpm *.dds *.fax *.g3 *.sgi *.exr *.pct *.pic *.pict *.webp *.jxr *.mrw *.raf *.mef *.raw *.orf *.djvu *.or2 *.icns *.dng *.svg *.nef *.pef *.pxm *.pnm)"]
        onAccepted: {
            openImageFile(fileDialog.fileUrls[0])
        }
    }

    FolderListModel
    {
        id: foldermodel
        folder: "file://" + escape(platform.picturesLocation()) //不明确这个模块的作用,暂时使用这种方式来转换字符串和URL
        showDirs: false
        showDotAndDotDot: false
        nameFilters: ["*.dng", "*.nef", "*.bmp", "*.gif", "*.ico", "*.jpeg", "*.jpg", "*.pbm", "*.pgm","*.png",  "*.pnm", "*.ppm",
            "*.svg", "*.tga", "*.tif", "*.tiff", "*.wbmp", "*.webp", "*.xbm", "*.xpm", "*.gif"]
        sortField: FolderListModel.Type
        showOnlyReadable: true
        sortReversed: false
    }

    Component.onCompleted: {
        var tempPath = fileControl.parseCommandlineGetPath("x")
        openImageFile(tempPath)
    }

}
