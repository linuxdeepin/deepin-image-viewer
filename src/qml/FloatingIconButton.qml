// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as D

// 浮动图标按钮：提供半透明磨砂玻璃风格背景，
// 使按钮图标在任意亮度的图片背景上都保持可见。
// 解决原 FloatingButton 图标跟随系统主题而非图片底色导致的可见性问题。
//
// 注意：不使用 D.FloatingPanel（InWindowBlur），因为 InWindowBlur 在窗口背景层渲染，
// 会被不透明的图片内容遮挡。改用 AbstractButton + Rectangle 背景，
// 在正常 QML z 序上渲染，确保始终覆盖在图片之上。
//
// 基于 D.AbstractButton（而非 D.Control），因为 AbstractButton 提供了
// pressed/clicked 信号和 checked 状态，而 Control 仅有 hovered。
D.AbstractButton {
    id: control

    // 按钮图标名称
    property string iconName: ""

    // 背景调色板：用 property 声明，使 ColorSelector 能解析 hovered/pressed 槽
    property D.Palette highlightBgColor: D.Palette {
        normal: D.DTK.makeColor(D.Color.Highlight)
        normalDark: D.DTK.makeColor(D.Color.Highlight)
        hovered: D.DTK.makeColor(D.Color.Highlight).lightness(+10)
        hoveredDark: D.DTK.makeColor(D.Color.Highlight).lightness(+10)
        pressed: D.DTK.makeColor(D.Color.Highlight).lightness(-10)
        pressedDark: D.DTK.makeColor(D.Color.Highlight).lightness(-10)
    }
    property D.Palette glassBgColor: D.Palette {
        // normal 使用 DTK behindWindowBlur 标准色（浅色 60% 不透明浅灰，深色 33% 不透明黑）
        normal: Qt.rgba(235 / 255, 235 / 255, 235 / 255, 0.6)
        normalDark: Qt.rgba(0, 0, 0, 0.33)
        // hovered/pressed 提高不透明度产生"玻璃变实"的交互反馈
        hovered: Qt.rgba(235 / 255, 235 / 255, 235 / 255, 0.8)
        hoveredDark: Qt.rgba(0, 0, 0, 0.5)
        pressed: Qt.rgba(235 / 255, 235 / 255, 235 / 255, 0.9)
        pressedDark: Qt.rgba(0, 0, 0, 0.6)
    }

    implicitWidth: 50
    implicitHeight: 50

    // 启用内置 hover 事件处理
    hoverEnabled: true

    D.ColorSelector.hovered: control.hovered
    D.ColorSelector.pressed: control.pressed

    // 半透明圆形背景：替代 InWindowBlur，在正常 z 序渲染，始终覆盖图片。
    // D.Palette 通过 ColorSelector 自动根据 hovered/pressed/checked 状态切换色值。
    background: Rectangle {
        radius: control.width / 2
        color: control.checked ? control.D.ColorSelector.highlightBgColor
                                : control.D.ColorSelector.glassBgColor
        border.width: 1
        border.color: control.checked ? Qt.rgba(255, 255, 255, 0.3)
                                       : Qt.rgba(0, 0, 0, 0.1)
    }

    D.DciIcon {
        anchors.centerIn: parent
        height: 45
        width: 45
        name: control.iconName
        palette: control.checked ? D.DTK.makeIconPalette(highlightTextPalette)
                                 : D.DTK.makeIconPalette(control.palette)
    }

    D.Palette {
        id: highlightTextPalette
        normal: D.DTK.makeColor(D.Color.HighlightedText)
        normalDark: D.DTK.makeColor(D.Color.HighlightedText)
    }
}
