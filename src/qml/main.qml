// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml 2.11
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

ApplicationWindow {
    id: window

    property bool isFullScreen: window.visibility === Window.FullScreen

    signal sigTitlePress

    // 设置 dtk 风格窗口
    DWindow.enabled: true
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    height: IV.FileControl.getlastHeight()
    minimumHeight: IV.GStatus.minHeight
    minimumWidth: IV.GStatus.minWidth
    visible: true
    width: IV.FileControl.getlastWidth()

    Component.onCompleted: {
        if (IV.FileControl.isCheckOnly()) {
            setX(screen.width / 2 - width / 2);
            setY(screen.height / 2 - height / 2);
        }
    }
    onClosing: {
        IV.FileControl.saveSetting(); //保存信息
        IV.FileControl.terminateShortcutPanelProcess(); //结束快捷键面板进程
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
        }

        target: IV.GControl
    }
}
