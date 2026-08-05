// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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

    Accessible.name: "ImageViewer"
    Accessible.role: Accessible.Application

    property bool isFullScreen: window.visibility === Window.FullScreen

    signal sigTitlePress

    // Bug fix: 使用 ListView 替换 PathView 时，出现内部的 mouseArea 鼠标操作会被 DWindow 截取
    // 导致 flicking 时拖动窗口，此处使用此标志禁用此行为
    DWindow.enableSystemMove: !IV.GStatus.viewFlicking

    // 设置 dtk 风格窗口
    DWindow.enabled: true
    // 设置 DTK 调色板：使窗口按钮 DCI 图标 (WindowButton/DciIcon) 的 ColorSelector 上下文正确，
    // 按 press/hover 状态渲染 DTK 标准蓝色。缺少此项时 DTK 控件继承 Qt 默认 palette，
    // ColorSelector 上下文断裂，图标 press 态无法变蓝。对齐 D.ApplicationWindow 的 palette 行为。
    palette: DTK.palette
    // 调整暗色主题下的窗口背景色
    color: DS.Style.control.selectColor(palette.window, palette.window, Qt.rgba(24 / 255, 24 / 255, 24 / 255, 1))
    // uos-design: allow-overlay-titlebar 应用采用沉浸式浮动标题栏 (ViewTopTitle) 而非窗口级 D.TitleBar header，
    // 标题栏随图片缩放/全屏滑入滑出，窗口按钮 (菜单/最小化/最大化/关闭) 由浮动 TitleBar 内的
    // D.WindowButtonGroup 提供，符合 DTK 窗口按钮 hover/press 标准实现。
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint | Qt.WindowCloseButtonHint
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
        }

        target: IV.GControl
    }
}
