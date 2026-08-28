// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_globalstatus.h"
#include "globalstatus.h"
#include "types.h"

#include <QSignalSpy>

void ut_globalstatus::SetUp()
{
}

void ut_globalstatus::TearDown()
{
}

// 测试常量属性返回值
TEST_F(ut_globalstatus, ConstantProperties)
{
    GlobalStatus status;

    EXPECT_EQ(status.minHeight(), 300);
    EXPECT_EQ(status.minWidth(), 628);
    EXPECT_EQ(status.minHideHeight(), 425);
    EXPECT_EQ(status.floatMargin(), 65);
    EXPECT_EQ(status.titleHeight(), 50);
    EXPECT_EQ(status.thumbnailViewHeight(), 70);
    EXPECT_EQ(status.showBottomY(), 80);
    EXPECT_EQ(status.switchImageHotspotWidth(), 100);
    EXPECT_EQ(status.actionMargin(), 9);
    EXPECT_EQ(status.rightMenuItemHeight(), 32);
    EXPECT_DOUBLE_EQ(status.animationDefaultDuration(), 366.0);
    EXPECT_EQ(status.pathViewItemCount(), 3);
}

// 测试 showFullScreen 属性设置与信号
TEST_F(ut_globalstatus, ShowFullScreenProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.showFullScreen());

    QSignalSpy spy(&status, &GlobalStatus::showFullScreenChanged);
    status.setShowFullScreen(true);
    EXPECT_TRUE(status.showFullScreen());
    EXPECT_EQ(spy.count(), 1);

    // 设置相同值不应触发信号
    status.setShowFullScreen(true);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 enableNavigation 属性默认值与设置
TEST_F(ut_globalstatus, EnableNavigationProperty)
{
    GlobalStatus status;
    EXPECT_TRUE(status.enableNavigation());

    QSignalSpy spy(&status, &GlobalStatus::enableNavigationChanged);
    status.setEnableNavigation(false);
    EXPECT_FALSE(status.enableNavigation());
    EXPECT_EQ(spy.count(), 1);
}

// 测试 stackPage 属性设置
TEST_F(ut_globalstatus, StackPageProperty)
{
    GlobalStatus status;
    EXPECT_EQ(status.stackPage(), Types::OpenImagePage);

    QSignalSpy spy(&status, &GlobalStatus::stackPageChanged);
    status.setStackPage(Types::ImageViewPage);
    EXPECT_EQ(status.stackPage(), Types::ImageViewPage);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 thumbnailVaildWidth 属性设置
TEST_F(ut_globalstatus, ThumbnailVaildWidthProperty)
{
    GlobalStatus status;
    EXPECT_EQ(status.thumbnailVaildWidth(), 0);

    QSignalSpy spy(&status, &GlobalStatus::thumbnailVaildWidthChanged);
    status.setThumbnailVaildWidth(200);
    EXPECT_EQ(status.thumbnailVaildWidth(), 200);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 delayInit 属性设置
TEST_F(ut_globalstatus, DelayInitProperty)
{
    GlobalStatus status;
    EXPECT_TRUE(status.delayInit());

    QSignalSpy spy(&status, &GlobalStatus::delayInitChanged);
    status.setDelayInit(false);
    EXPECT_FALSE(status.delayInit());
    EXPECT_EQ(spy.count(), 1);

    // 设置相同值不应触发信号
    status.setDelayInit(false);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 showRightMenu 属性设置与信号
TEST_F(ut_globalstatus, ShowRightMenuProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.showRightMenu());

    QSignalSpy spy(&status, &GlobalStatus::showRightMenuChanged);
    status.setShowRightMenu(true);
    EXPECT_TRUE(status.showRightMenu());
    EXPECT_EQ(spy.count(), 1);

    // 设置相同值不应触发信号
    status.setShowRightMenu(true);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 showImageInfo 属性设置与信号
TEST_F(ut_globalstatus, ShowImageInfoProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.showImageInfo());

    QSignalSpy spy(&status, &GlobalStatus::showImageInfoChanged);
    status.setShowImageInfo(true);
    EXPECT_TRUE(status.showImageInfo());
    EXPECT_EQ(spy.count(), 1);

    status.setShowImageInfo(true);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 viewInteractive 属性（默认 true）设置与信号
TEST_F(ut_globalstatus, ViewInteractiveProperty)
{
    GlobalStatus status;
    EXPECT_TRUE(status.viewInteractive());

    QSignalSpy spy(&status, &GlobalStatus::viewInteractiveChanged);
    status.setViewInteractive(false);
    EXPECT_FALSE(status.viewInteractive());
    EXPECT_EQ(spy.count(), 1);

    status.setViewInteractive(false);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 viewFlicking 属性设置与信号
TEST_F(ut_globalstatus, ViewFlickingProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.viewFlicking());

    QSignalSpy spy(&status, &GlobalStatus::viewFlickingChanged);
    status.setViewFlicking(true);
    EXPECT_TRUE(status.viewFlicking());
    EXPECT_EQ(spy.count(), 1);

    status.setViewFlicking(true);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 animationBlock 属性设置与信号
TEST_F(ut_globalstatus, AnimationBlockProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.animationBlock());

    QSignalSpy spy(&status, &GlobalStatus::animationBlockChanged);
    status.setAnimationBlock(true);
    EXPECT_TRUE(status.animationBlock());
    EXPECT_EQ(spy.count(), 1);

    status.setAnimationBlock(true);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 fullScreenAnimating 属性设置与信号
TEST_F(ut_globalstatus, FullScreenAnimatingProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.fullScreenAnimating());

    QSignalSpy spy(&status, &GlobalStatus::fullScreenAnimatingChanged);
    status.setFullScreenAnimating(true);
    EXPECT_TRUE(status.fullScreenAnimating());
    EXPECT_EQ(spy.count(), 1);

    status.setFullScreenAnimating(true);
    EXPECT_EQ(spy.count(), 1);
}

// 析构函数: 触发 D0 deleting destructor (new + delete)
TEST_F(ut_globalstatus, Destructor_DeletingDestructor)
{
    auto *obj = new GlobalStatus();
    delete obj;
    SUCCEED();
}

// 测试 editMode 属性设置与信号
TEST_F(ut_globalstatus, EditModeProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.editMode());

    QSignalSpy spy(&status, &GlobalStatus::editModeChanged);
    status.setEditMode(true);
    EXPECT_TRUE(status.editMode());
    EXPECT_EQ(spy.count(), 1);

    // 设置相同值不应触发信号
    status.setEditMode(true);
    EXPECT_EQ(spy.count(), 1);

    status.setEditMode(false);
    EXPECT_FALSE(status.editMode());
    EXPECT_EQ(spy.count(), 2);
}

// 测试 editModified 属性设置与信号
TEST_F(ut_globalstatus, EditModifiedProperty)
{
    GlobalStatus status;
    EXPECT_FALSE(status.editModified());

    QSignalSpy spy(&status, &GlobalStatus::editModifiedChanged);
    status.setEditModified(true);
    EXPECT_TRUE(status.editModified());
    EXPECT_EQ(spy.count(), 1);

    // 设置相同值不应触发信号
    status.setEditModified(true);
    EXPECT_EQ(spy.count(), 1);

    status.setEditModified(false);
    EXPECT_FALSE(status.editModified());
    EXPECT_EQ(spy.count(), 2);
}
