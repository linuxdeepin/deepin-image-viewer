// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import org.deepin.image.viewer 1.0 as IV

ApplicationWindow {
    id: window

    property bool isFullScreen: window.visibility === Window.FullScreen

    signal sigTitlePress

    // Bug fix: 使用 ListView 替换 PathView 时，出现内部的 mouseArea 鼠标操作会被 DWindow 截取
    // 导致 flicking 时拖动窗口，此处使用此标志禁用此行为
    DWindow.enableSystemMove: !IV.GStatus.viewFlicking

    // 设置 dtk 风格窗口
    DWindow.enabled: true
    // 调整暗色主题下的窗口背景色
    color: DS.Style.control.selectColor(palette.window, palette.window, Qt.rgba(24 / 255, 24 / 255, 24 / 255, 1))
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    height: IV.FileControl.getlastHeight()
    minimumHeight: IV.GStatus.minHeight
    minimumWidth: IV.GStatus.minWidth
    visible: true
    width: IV.FileControl.getlastWidth()

    Component.onCompleted: {
        if (IV.FileControl.isCheckOnly()) {
            setX(IV.FileControl.getPrimaryScreenCenterX(width));
            setY(IV.FileControl.getPrimaryScreenCenterY(height));
        }
        // 如果当前有图片源，手动设置窗口标题
        if (IV.GControl.currentSource.toString() !== "") {
            window.title = IV.FileControl.slotGetFileName(IV.GControl.currentSource) + IV.FileControl.slotFileSuffix(IV.GControl.currentSource);
        }
    }
    onClosing: {
        IV.FileControl.saveSetting(); //保存信息
        IV.FileControl.terminateShortcutPanelProcess(); //结束快捷键面板进程
        IV.GControl.forceExit();
    }
    onHeightChanged: {
        if (window.visibility != Window.FullScreen && window.visibility != Window.Maximized) {
            IV.FileControl.setSettingHeight(height);
        }
    }
    onWidthChanged: {
        if (window.visibility != Window.FullScreen && window.visibility != Window.Maximized) {
            IV.FileControl.setSettingWidth(width);
        }
    }

    MainStack {
        anchors.fill: parent
    }

    Connections {
        function onCurrentSourceChanged() {
            window.title = IV.FileControl.slotGetFileName(IV.GControl.currentSource) + IV.FileControl.slotFileSuffix(IV.GControl.currentSource);
            if (window.visibility !== Window.FullScreen && window.visibility !== Window.Maximized) {
                setX(IV.FileControl.getPrimaryScreenCenterX(width));
                setY(IV.FileControl.getPrimaryScreenCenterY(height));
            }
        }

        target: IV.GControl
    }
}
