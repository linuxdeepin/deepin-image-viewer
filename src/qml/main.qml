// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml 2.11
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

ApplicationWindow {
    id: window

    property bool isFullScreen: window.visibility === Window.FullScreen

    signal sigTitlePress()

    // 设置 dtk 风格窗口
    DWindow.enabled: true
    visible: true
    minimumHeight: GStatus.minHeight
    minimumWidth: GStatus.minWidth
    width: fileControl.getlastWidth()
    height: fileControl.getlastHeight()
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

    onWidthChanged: {
        if (window.visibility != Window.FullScreen
                && window.visibility != Window.Maximized) {
            fileControl.setSettingWidth(width)
        }
    }

    onHeightChanged: {
        if (window.visibility != Window.FullScreen
                && window.visibility != Window.Maximized) {
            fileControl.setSettingHeight(height)
        }
    }

    onClosing: {
        fileControl.saveSetting() //保存信息
        fileControl.terminateShortcutPanelProcess() //结束快捷键面板进程
    }

    MainStack {
        anchors.fill: parent
    }

    Connections {
        target: GControl
        onCurrentSourceChanged: {
            window.title = fileControl.slotGetFileName(GControl.currentSource) + fileControl.slotFileSuffix(GControl.currentSource)
        }
    }

    Component.onCompleted: {
        if (fileControl.isCheckOnly()) {
            setX(screen.width / 2 - width / 2)
            setY(screen.height / 2 - height / 2)
        }
    }
}
