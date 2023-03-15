// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalstatus.h"

enum ConstProperty {
    MinHeight = 300,
    MinWidth = 628,
    MinHideHeight = 425,  //调整窗口高度小于425px时，隐藏工具栏和标题栏
    FloatMargin = 65,
    TitleHeight = 50,
    ThumbnailViewHeight = 70,       // 底部工具栏高度 70px
    ShowBottomY = 80,               // 底部工具栏显示时距离底部的高度 80px (工具栏高度 70px + 边距 10px)
    SwitchImageHotspotWidth = 100,  // 左右切换图片按钮的热区宽度 100px
    ActionMargin = 9,               // 应用图标距离顶栏
    RightMenuItemHeight = 32,       // 右键菜单item的高度
};

/**
   @class GlobalStatus
   @brief QML单例类，维护全局状态，同步不同组件间的状态信息
   @details 相较于使用脚本配置的 program Singletion , Qt 更推崇使用 QObject 注册单例
   @link https://doc.qt.io/qt-6/qtquick-performance.html#use-singleton-types-instead-of-pragma-library-scripts
 */

GlobalStatus::GlobalStatus(QObject *parent)
    : QObject(parent)
{
}

GlobalStatus::~GlobalStatus() {}

bool GlobalStatus::isShowRightMenu() const
{
    return showRightMenuDialog;
}

void GlobalStatus::setShowRightMenu(bool b)
{
    if (showRightMenuDialog != b) {
        showRightMenuDialog = b;
        Q_EMIT showRightMenuChanged();
    }
}

bool GlobalStatus::isShowImageInfo() const
{
    return showImageInfoDialog;
}

void GlobalStatus::setShowImageInfo(bool b)
{
    if (showImageInfoDialog != b) {
        showImageInfoDialog = b;
        Q_EMIT showImageInfoChanged();
    }
}

bool GlobalStatus::viewInteractive() const
{
    return storeViewInteractive;
}

void GlobalStatus::setViewInterActive(bool b)
{
    if (storeViewInteractive != b) {
        storeViewInteractive = b;
        Q_EMIT viewInteractiveChanged();
    }
}

bool GlobalStatus::viewFlicking() const
{
    return storeviewFlicking;
}

void GlobalStatus::setViewFlicking(bool b)
{
    if (storeviewFlicking != b) {
        storeviewFlicking = b;
        Q_EMIT viewFlickingChanged();
    }
}

void GlobalStatus::setThumbnailVaildWidth(int width)
{
    if (storeThumbnailVaildWidth != width) {
        storeThumbnailVaildWidth = width;
        Q_EMIT thumbnailVaildWidthChanged();
    }
}

int GlobalStatus::thumbnailVaildWidth() const
{
    return storeThumbnailVaildWidth;
}

int GlobalStatus::minHeight() const
{
    return MinHeight;
}

int GlobalStatus::minWidth() const
{
    return MinWidth;
}

int GlobalStatus::minHideHeight() const
{
    return MinHideHeight;
}

int GlobalStatus::floatMargin() const
{
    return FloatMargin;
}

int GlobalStatus::titleHeight() const
{
    return TitleHeight;
}

int GlobalStatus::thumbnailViewHeight() const
{
    return ThumbnailViewHeight;
}

int GlobalStatus::showBottomY() const
{
    return ShowBottomY;
}

int GlobalStatus::switchImageHotspotWidth() const
{
    return SwitchImageHotspotWidth;
}

int GlobalStatus::actionMargin() const
{
    return ActionMargin;
}

int GlobalStatus::rightMenuItemHeight() const
{
    return RightMenuItemHeight;
}
