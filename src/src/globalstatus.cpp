// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalstatus.h"
#include <DLog>
#include "types.h"

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

static const int sc_MinHeight = 300;           // 窗口最小高度
static const int sc_MinWidth = 628;            // 窗口最小宽度
static const int sc_MinHideHeight = 425;       // 调整窗口高度小于425px时，隐藏工具栏和标题栏
static const int sc_FloatMargin = 65;          // 浮动按钮边距
static const int sc_TitleHeight = 50;          // 标题栏栏高度
static const int sc_ThumbnailViewHeight = 70;  // 底部工具栏高度 70px
static const int sc_ShowBottomY = 80;  // 底部工具栏显示时距离底部的高度 80px (工具栏高度 70px + 边距 10px)
static const int sc_SwitchImageHotspotWidth = 100;  // 左右切换图片按钮的热区宽度 100px
static const int sc_ActionMargin = 9;               // 应用图标距离顶栏
static const int sc_RightMenuItemHeight = 32;       // 右键菜单item的高度

static const double sc_AnimationDefaultDuration = 366;  // 默认动画时长
static const int sc_PathViewItemCount = 3;              // 默认 PathView 在路径中的 Item 计数

/**
   @class GlobalStatus
   @brief QML单例类，维护全局状态，同步不同组件间的状态信息
   @details 相较于使用脚本配置的 program Singletion , Qt 更推崇使用 QObject 注册单例
   @link https://doc.qt.io/qt-6/qtquick-performance.html#use-singleton-types-instead-of-pragma-library-scripts
 */

GlobalStatus::GlobalStatus(QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "GlobalStatus constructor called.";
}

GlobalStatus::~GlobalStatus() {
    qCDebug(logImageViewer) << "GlobalStatus destructor called.";
}

/**
   @return 返回是否全屏显示图片
 */
bool GlobalStatus::showFullScreen() const
{
    qCDebug(logImageViewer) << "GlobalStatus::showFullScreen() called, returning: " << storeshowFullScreen;
    return storeshowFullScreen;
}

/**
   @brief 设置全屏显示图片
 */
void GlobalStatus::setShowFullScreen(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setShowFullScreen() called with value: " << value;
    if (value != storeshowFullScreen) {
        storeshowFullScreen = value;
        Q_EMIT showFullScreenChanged();
        qCDebug(logImageViewer) << "showFullScreen changed to: " << storeshowFullScreen << ", emitting showFullScreenChanged.";
    }
}

/**
   @return 返回是否允许显示导航窗口
 */
bool GlobalStatus::enableNavigation() const
{
    qCDebug(logImageViewer) << "GlobalStatus::enableNavigation() called, returning: " << storeenableNavigation;
    return storeenableNavigation;
}

/**
   @brief 设置是否允许显示导航窗口
 */
void GlobalStatus::setEnableNavigation(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setEnableNavigation() called with value: " << value;
    if (value != storeenableNavigation) {
        storeenableNavigation = value;
        Q_EMIT enableNavigationChanged();
        qCDebug(logImageViewer) << "enableNavigation changed to: " << storeenableNavigation << ", emitting enableNavigationChanged.";
    }
}

/**
   @return 返回是否显示右键菜单
 */
bool GlobalStatus::showRightMenu() const
{
    qCDebug(logImageViewer) << "GlobalStatus::showRightMenu() called, returning: " << storeshowRightMenu;
    return storeshowRightMenu;
}

/**
   @brief 设置是否显示右键菜单
 */
void GlobalStatus::setShowRightMenu(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setShowRightMenu() called with value: " << value;
    if (value != storeshowRightMenu) {
        storeshowRightMenu = value;
        Q_EMIT showRightMenuChanged();
        qCDebug(logImageViewer) << "showRightMenu changed to: " << storeshowRightMenu << ", emitting showRightMenuChanged.";
    }
}

/**
   @return 当前是否弹窗显示详细图像信息
 */
bool GlobalStatus::showImageInfo() const
{
    qCDebug(logImageViewer) << "GlobalStatus::showImageInfo() called, returning: " << storeshowImageInfo;
    return storeshowImageInfo;
}

/**
   @brief 设置是否弹窗显示详细图像信息
 */
void GlobalStatus::setShowImageInfo(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setShowImageInfo() called with value: " << value;
    if (value != storeshowImageInfo) {
        storeshowImageInfo = value;
        Q_EMIT showImageInfoChanged();
        qCDebug(logImageViewer) << "showImageInfo changed to: " << storeshowImageInfo << ", emitting showImageInfoChanged.";
    }
}

/**
   @return 返回滑动视图是否响应操作
 */
bool GlobalStatus::viewInteractive() const
{
    qCDebug(logImageViewer) << "GlobalStatus::viewInteractive() called, returning: " << storeviewInteractive;
    return storeviewInteractive;
}

/**
   @brief 设置滑动视图是否响应操作
 */
void GlobalStatus::setViewInteractive(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setViewInteractive() called with value: " << value;
    if (value != storeviewInteractive) {
        storeviewInteractive = value;
        Q_EMIT viewInteractiveChanged();
        qCDebug(logImageViewer) << "viewInteractive changed to: " << storeviewInteractive << ", emitting viewInteractiveChanged.";
    }
}

/**
   @return 返回滑动视图是否处于轻弹状态
 */
bool GlobalStatus::viewFlicking() const
{
    qCDebug(logImageViewer) << "GlobalStatus::viewFlicking() called, returning: " << storeviewFlicking;
    return storeviewFlicking;
}

/**
   @brief 设置当前滑动视图是否处于轻弹状态
 */
void GlobalStatus::setViewFlicking(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setViewFlicking() called with value: " << value;
    if (value != storeviewFlicking) {
        storeviewFlicking = value;
        Q_EMIT viewFlickingChanged();
        qCDebug(logImageViewer) << "viewFlicking changed to: " << storeviewFlicking << ", emitting viewFlickingChanged.";
    }
}

/**
   @return 返回当前是否允许标题栏、底栏动画效果
 */
bool GlobalStatus::animationBlock() const
{
    qCDebug(logImageViewer) << "GlobalStatus::animationBlock() called, returning: " << storeanimationBlock;
    return storeanimationBlock;
}

/**
   @brief 设置当前允许标题栏、底栏动画效果的标志值为 \a value
 */
void GlobalStatus::setAnimationBlock(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setAnimationBlock() called with value: " << value;
    if (value != storeanimationBlock) {
        storeanimationBlock = value;
        Q_EMIT animationBlockChanged();
        qCDebug(logImageViewer) << "animationBlock changed to: " << storeanimationBlock << ", emitting animationBlockChanged.";
    }
}

/**
   @return 返回当前是否允许全屏展示动画
 */
bool GlobalStatus::fullScreenAnimating() const
{
    qCDebug(logImageViewer) << "GlobalStatus::fullScreenAnimating() called, returning: " << storefullScreenAnimating;
    return storefullScreenAnimating;
}

/**
   @brief 设置当前是否允许全屏展示动画的标志值为 \a value
 */
void GlobalStatus::setFullScreenAnimating(bool value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setFullScreenAnimating() called with value: " << value;
    if (value != storefullScreenAnimating) {
        storefullScreenAnimating = value;
        Q_EMIT fullScreenAnimatingChanged();
        qCDebug(logImageViewer) << "fullScreenAnimating changed to: " << storefullScreenAnimating << ", emitting fullScreenAnimatingChanged.";
    }
}

/**
   @return 返回当前缩略图列表允许显示的宽度
 */
int GlobalStatus::thumbnailVaildWidth() const
{
    qCDebug(logImageViewer) << "GlobalStatus::thumbnailVaildWidth() called, returning: " << storethumbnailVaildWidth;
    return storethumbnailVaildWidth;
}

/**
   @brief 设置当前缩略图列表允许显示的宽度为 \a value
 */
void GlobalStatus::setThumbnailVaildWidth(int value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setThumbnailVaildWidth() called with value: " << value;
    if (value != storethumbnailVaildWidth) {
        storethumbnailVaildWidth = value;
        Q_EMIT thumbnailVaildWidthChanged();
        qCDebug(logImageViewer) << "thumbnailVaildWidth changed to: " << storethumbnailVaildWidth << ", emitting thumbnailVaildWidthChanged.";
    }
}

/**
   @return 返回当前显示的界面索引
 */
Types::StackPage GlobalStatus::stackPage() const
{
    qCDebug(logImageViewer) << "GlobalStatus::stackPage() called, returning: " << storestackPage;
    return storestackPage;
}

/**
   @brief 设置当前显示的界面索引为 \a value ，将切换显示的界面类型
 */
void GlobalStatus::setStackPage(Types::StackPage value)
{
    qCDebug(logImageViewer) << "GlobalStatus::setStackPage() called with value: " << value;
    if (value != storestackPage) {
        storestackPage = value;
        Q_EMIT stackPageChanged();
        qCDebug(logImageViewer) << "stackPage changed to: " << storestackPage << ", emitting stackPageChanged.";
    }
}

bool GlobalStatus::delayInit() const
{
    qCDebug(logImageViewer) << "GlobalStatus::delayInit() called, returning: " << storeDelayInit;
    return storeDelayInit;
}

void GlobalStatus::setDelayInit(bool b)
{
    qCDebug(logImageViewer) << "GlobalStatus::setDelayInit() called with value: " << b;
    if (storeDelayInit != b) {
        storeDelayInit = b;
        Q_EMIT delayInitChanged();
        qCDebug(logImageViewer) << "delayInit changed to: " << storeDelayInit << ", emitting delayInitChanged.";
    }
}

int GlobalStatus::minHeight() const
{
    qCDebug(logImageViewer) << "GlobalStatus::minHeight() called, returning: " << sc_MinHeight;
    return sc_MinHeight;
}

int GlobalStatus::minWidth() const
{
    qCDebug(logImageViewer) << "GlobalStatus::minWidth() called, returning: " << sc_MinWidth;
    return sc_MinWidth;
}

int GlobalStatus::minHideHeight() const
{
    qCDebug(logImageViewer) << "GlobalStatus::minHideHeight() called, returning: " << sc_MinHideHeight;
    return sc_MinHideHeight;
}

int GlobalStatus::floatMargin() const
{
    qCDebug(logImageViewer) << "GlobalStatus::floatMargin() called, returning: " << sc_FloatMargin;
    return sc_FloatMargin;
}

int GlobalStatus::titleHeight() const
{
    qCDebug(logImageViewer) << "GlobalStatus::titleHeight() called, returning: " << sc_TitleHeight;
    return sc_TitleHeight;
}

int GlobalStatus::thumbnailViewHeight() const
{
    qCDebug(logImageViewer) << "GlobalStatus::thumbnailViewHeight() called, returning: " << sc_ThumbnailViewHeight;
    return sc_ThumbnailViewHeight;
}

int GlobalStatus::showBottomY() const
{
    qCDebug(logImageViewer) << "GlobalStatus::showBottomY() called, returning: " << sc_ShowBottomY;
    return sc_ShowBottomY;
}

int GlobalStatus::switchImageHotspotWidth() const
{
    qCDebug(logImageViewer) << "GlobalStatus::switchImageHotspotWidth() called, returning: " << sc_SwitchImageHotspotWidth;
    return sc_SwitchImageHotspotWidth;
}

int GlobalStatus::actionMargin() const
{
    qCDebug(logImageViewer) << "GlobalStatus::actionMargin() called, returning: " << sc_ActionMargin;
    return sc_ActionMargin;
}

int GlobalStatus::rightMenuItemHeight() const
{
    qCDebug(logImageViewer) << "GlobalStatus::rightMenuItemHeight() called, returning: " << sc_RightMenuItemHeight;
    return sc_RightMenuItemHeight;
}

double GlobalStatus::animationDefaultDuration() const
{
    qCDebug(logImageViewer) << "GlobalStatus::animationDefaultDuration() called, returning: " << sc_AnimationDefaultDuration;
    return sc_AnimationDefaultDuration;
}

/**
   @brief 默认 PathView 在路径中的 Item 计数
   @note 会影响 PathView 相关的动画效果计算，修改此值需慎重考虑
 */
int GlobalStatus::pathViewItemCount() const
{
    qCDebug(logImageViewer) << "GlobalStatus::pathViewItemCount() called, returning: " << sc_PathViewItemCount;
    return sc_PathViewItemCount;
}
