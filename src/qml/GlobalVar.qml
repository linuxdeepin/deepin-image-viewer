// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

Item {
    property int minHeight: 300
    property int minWidth: 628
    property int minHideHeight: 425 //调整窗口高度小于425px时，隐藏工具栏和标题栏
    property int floatMargin: 65
    property int titleHeight: 50
    property int thumbnailViewHeight: 70 // 底部工具栏高度 70px
    property int showBottomY: 80 // 底部工具栏显示时距离底部的高度 80px (工具栏高度 70px + 边距 10px)
    property int switchImageHotspotWidth: 100 // 左右切换图片按钮的热区宽度 100px
    property int actionMargin: 9 // 应用图标距离顶栏
    property int rightMenuItemHeight: 32 // 右键菜单item的高度
    property bool animationBlock: false

    signal sigWindowStateChange()
}
