// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV
import "./Dialog"

Item {
    id: stackView
    Accessible.name: IV.GStatus.stackPage === Number(IV.Types.ImageViewPage) ? qsTr("Image view") : qsTr("Open image")
    Accessible.role: Accessible.Pane

    signal saveEditCopyRequested

    property url pendingSourcePath: ""

    function cancelPendingSourceChange() {
        pendingSourcePath = "";
    }

    function completePendingSourceChange() {
        if (pendingSourcePath.toString() === "") return;
        var path = pendingSourcePath;
        pendingSourcePath = "";
        setSourcePath(path);
    }

    function requestSourcePath(path) {
        if (path.toString() === IV.GControl.currentSource.toString()) return;
        if (!IV.GStatus.editMode) {
            setSourcePath(path);
            return;
        }

        if (pendingSourcePath.toString() !== "") {
            pendingSourcePath = path;
            return;
        }

        pendingSourcePath = path;
        if (IV.GStatus.editModified) {
            unsavedEditDialog.open();
        } else {
            IV.GStatus.editMode = false;
            completePendingSourceChange();
        }
    }

    function requestExitEditMode() {
        if (!IV.GStatus.editMode) {
            return;
        }
        if (IV.GStatus.editModified) {
            unsavedEditDialog.open();
        } else {
            IV.GStatus.editMode = false;
        }
    }

    function saveEditCopyAndExit() {
        if (contentLoader.item && contentLoader.item.openSaveDialog)
            contentLoader.item.openSaveDialog(true);
    }

    // 打开图片对话框
    function openImageDialog() {
        if (Loader.Ready === fileDialogLoader.status) {
            fileDialogLoader.item.open();
        } else {
            fileDialogLoader.active = true;
        }
    }

    // 设置当前使用的图片源
    function setSourcePath(path) {
        if (IV.FileControl.isCurrentWatcherDir(path)) {
            if (IV.GControl.globalModel.indexForImagePath(path) === -1) {
                if (IV.GControl.addImageAndSetCurrentSource(path)) {
                    IV.FileControl.addImageFile(path.toString());
                }
            } else {
                // 更新当前文件路径
                IV.GControl.currentSource = path;
            }
        } else {
            var sourcePaths = IV.FileControl.getDirImagePath(path);
            if (sourcePaths.length > 0) {
                if (!IV.GControl.setImageFiles(sourcePaths, path)) {
                    console.warn("Rejected image path not in image list", path);
                    return;
                }
                // 记录当前读取的图片信息
                IV.FileControl.resetImageFiles(sourcePaths);
                console.log("Load image info", path);
                switchImageView();
            } else {
                switchOpenImage();
            }
        }
    }

    function switchImageView() {
        IV.GStatus.stackPage = Number(IV.Types.ImageViewPage);
        contentLoader.setSource("qrc:/qt/qml/IVModule/qml/FullImageView.qml");
    }

    function switchOpenImage() {
        IV.GStatus.stackPage = Number(IV.Types.OpenImagePage);
        window.title = "";
        contentLoader.setSource("qrc:/qt/qml/IVModule/qml/OpenImageWidget.qml");
    }

    function switchSliderShow() {
        if (Number(IV.Types.ImageViewPage) === IV.GStatus.stackPage) {
            IV.GStatus.stackPage = Number(IV.Types.SliderShowPage);
            contentLoader.setSource("qrc:/qt/qml/IVModule/qml/SliderShow.qml");
        }
    }

    anchors.fill: parent

    Component.onCompleted: {
        // main.cpp 从命令行启动时取得命令行参数，判断默认加载界面
        if (IV.GStatus.stackPage === Number(IV.Types.ImageViewPage)) {
            switchImageView();
        } else {
            switchOpenImage();
        }
    }

    Connections {
        // 关联外部通过 DBus 等方式触发调用看图
        function onOpenImageFile(fileName) {
            requestSourcePath(fileName);
        }

        target: IV.FileControl
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
        // Note: show at first
        asynchronous: true
    }

    DropArea {
        id: dropArea

        anchors.fill: parent

        onDropped: {
            if (drop.hasUrls && drop.urls.length !== 0
                    && IV.FileControl.isImage(drop.urls[0].toString())) {
                requestSourcePath(drop.urls[0]);
            }
        }
        onEntered: {
            background.color = "gray";
            drag.accept(Qt.CopyAction);
        }
        onExited: {
            background.color = "white";
            console.log("onExited");
        }
    }

    Loader {
        id: fileDialogLoader

        active: false
        asynchronous: true

        sourceComponent: FileDialog {
            id: fileDialog

            currentFolder: IV.FileControl.standardPicturesPath()
            fileMode: FileDialog.OpenFiles
            nameFilters: ["Image files (*.jpg *.png *.bmp *.gif *.ico *.jpe " + "*.jps *.jpeg *.jng *.koala *.koa *.lbm " + "*.iff *.mng *.pbm *.pbmraw *.pcd *.pcx " + "*.pgm *.pgmraw *.ppm *.ppmraw *.ras *.tga " + "*.targa *.tiff *.tif *.wbmp *.psd *.cut *.xbm " + "*.xpm *.dds *.fax *.g3 *.sgi *.exr *.pct *.pic " + "*.pict *.webp *.jxr *.mrw *.raf *.mef *.raw *.orf " + "*.djvu *.or2 *.icns *.dng *.svg *.nef *.pef *.pxm *.pnm *.avif *.heif *.heic)"]
            title: qsTr("Select pictures")

            Component.onCompleted: {
                fileDialog.open();
            }
            onAccepted: {
                stackView.requestSourcePath(fileDialog.selectedFiles[0]);
            }
        }
    }

    // 快捷键打开帮助手册
    Shortcut {
        autoRepeat: false
        enabled: true
        sequence: "F1"

        onActivated: {
            DTK.ApplicationHelper.handleHelpAction();
        }
    }

    // 打开图片文件
    Shortcut {
        sequence: "Ctrl+O"

        onActivated: {
            // 不在动画展示状态
            if (Number(IV.Types.SliderShowPage) !== IV.GStatus.stackPage) {
                openImageDialog();
            }
        }
    }

    // 进入图片编辑模式
    Shortcut {
        autoRepeat: false
        enabled: IV.GStatus.stackPage === Number(IV.Types.ImageViewPage)
                  && IV.GControl.currentSource.toString() !== ""
                  && IV.ImageEditor.canEdit(IV.GControl.currentSource)
                  && !IV.GStatus.editMode
        sequence: "Ctrl+Shift+E"

        onActivated: IV.GStatus.editMode = true
    }

    // 退出图片编辑模式；未保存状态确认由编辑会话接入后统一处理。
    Shortcut {
        autoRepeat: false
        enabled: IV.GStatus.editMode
        sequence: "Esc"

        onActivated: requestExitEditMode()
    }

    EditConfirmDialog {
        id: unsavedEditDialog

        property bool preservePendingSource: false

        title: ""
        message: qsTr("The current image has unsaved changes. Save?")
        parentWindow: Window.window
        secondaryMessage: qsTr("Image Modified")
        actions: [
            { "text": qsTr("Cancel"), "action": "cancel" },
            { "text": qsTr("Don't Save"), "action": "discard" },
            { "text": qsTr("Save Copy"), "action": "save", "recommended": true }
        ]

        onActionTriggered: function(action) {
            if (action === "discard") {
                IV.GStatus.editModified = false;
                IV.GStatus.editMode = false;
                stackView.completePendingSourceChange();
            } else if (action === "save") {
                preservePendingSource = true;
                stackView.saveEditCopyAndExit();
            } else {
                stackView.cancelPendingSourceChange();
            }
        }
        onClosing: function(close) {
            if (!preservePendingSource)
                stackView.cancelPendingSourceChange();
            preservePendingSource = false;
        }
    }

    // show shortcut panel
    Shortcut {
        sequence: "Ctrl+Shift+/"

        onActivated: {
            var screenPos = mapToGlobal(parent.x, parent.y);
            IV.FileControl.showShortcutPanel(screenPos.x + parent.Window.width / 2, screenPos.y + parent.Window.height / 2);
        }
    }
}
