// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALSTATUS_H
#define GLOBALSTATUS_H

#include "types.h"

#include <QObject>

// 定义使用的 QML 全局状态变量
#define GLOBAL_PROPERTY(T, X, DEFAULT_VAULE)                                                                                     \
public:                                                                                                                          \
    Q_PROPERTY(T X READ X WRITE set##X NOTIFY X##Changed)                                                                        \
    T X() const;                                                                                                                 \
    void set##X(T value);                                                                                                        \
    Q_SIGNAL void X##Changed();                                                                                                  \
                                                                                                                                 \
private:                                                                                                                         \
    T store##X = DEFAULT_VAULE;
// GLOBAL_PROPERTY

class GlobalStatus : public QObject
{
    Q_OBJECT
    // Constant properties.
    Q_PROPERTY(int minHeight READ minHeight CONSTANT)
    Q_PROPERTY(int minWidth READ minWidth CONSTANT)
    Q_PROPERTY(int minHideHeight READ minHideHeight CONSTANT)
    Q_PROPERTY(int floatMargin READ floatMargin CONSTANT)
    Q_PROPERTY(int titleHeight READ titleHeight CONSTANT)
    Q_PROPERTY(int thumbnailViewHeight READ thumbnailViewHeight CONSTANT)
    Q_PROPERTY(int showBottomY READ showBottomY CONSTANT)
    Q_PROPERTY(int switchImageHotspotWidth READ switchImageHotspotWidth CONSTANT)
    Q_PROPERTY(int actionMargin READ actionMargin CONSTANT)
    Q_PROPERTY(int rightMenuItemHeight READ rightMenuItemHeight CONSTANT)

public:
    explicit GlobalStatus(QObject *parent = nullptr);
    ~GlobalStatus() override;

    GLOBAL_PROPERTY(bool, showFullScreen, false)   // 切换全屏显示图片 (ImageViewer)
    GLOBAL_PROPERTY(bool, enableNavigation, true)  // 允许显示导航窗口
    GLOBAL_PROPERTY(bool, showRightMenu, false)    // 显示右键菜单
    GLOBAL_PROPERTY(bool, showImageInfo, false)    // 显示详细图像信息
    GLOBAL_PROPERTY(bool, viewInteractive, true)   // 滑动视图是否响应操作 (ImageViewer ListView)
    GLOBAL_PROPERTY(bool, viewFlicking, false)     // 滑动视图是否处于轻弹状态 (ImageViewer ListView)
    GLOBAL_PROPERTY(bool, animationBlock, false)   // 屏蔽标题栏/底部栏动画效果
    GLOBAL_PROPERTY(bool, fullScreenAnimating, false)  // 处于全屏动画状态标识，动画前后部分控件需重置，例如缩略图栏重新居中设置
    GLOBAL_PROPERTY(int, thumbnailVaildWidth, 0)                        // 缩略图列表允许的宽度
    GLOBAL_PROPERTY(Types::StackPage, stackPage, Types::OpenImagePage)  // 当前所处的界面索引

public:
    // Constant properties.
    int minHeight() const;
    int minWidth() const;
    int minHideHeight() const;
    int floatMargin() const;
    int titleHeight() const;
    int thumbnailViewHeight() const;
    int showBottomY() const;
    int switchImageHotspotWidth() const;
    int actionMargin() const;
    int rightMenuItemHeight() const;
};

#endif  // GLOBALSTATUS_H
