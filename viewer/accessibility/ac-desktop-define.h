/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作
#define OPENACCESSIBLE

#define OPEN_IMAGE QObject::tr("Open Image") // 打开图片按钮
#define NOT_FOUND_IMAGE QObject::tr("Image file not found") // 图片未找到lab

#define Thumbnail_Label QObject::tr("ThumbnailLabel") // 缩略图lab
#define Thumbnail_Widget QObject::tr("ThumbnailWidget") // 缩略图widegt

#define Lock_Widget QObject::tr("LockWidget") // 现实不支持格式或损坏图片窗口
#define WATER_MASK_TEXT_LABEL QObject::tr("water_mask_text") // 水印文字描述

#define ADAPT_BUTTON QObject::tr("AdaptBtn")//１：１视图
#define PRE_BUTTON QObject::tr("PreviousButton")//上一张
#define NEXT_BUTTON QObject::tr("NextButton")//下一张
#define ADAPT_SCREEN_BUTTON QObject::tr("AdaptScreenBtn") //适应屏幕按钮
#define CLOCKWISE_ROTATION QObject::tr("Clockwise rotation") //顺时针旋转
#define COUNTER_CLOCKWISE_ROTATION QObject::tr("Counter clockwise rotation") //逆时针旋转
#define TRASH_BUTTON QObject::tr("trashbtn") //删除按钮
#define TTBCONTENT_WIDGET QObject::tr("ttbcontent") //缩略图区域
#define VIEW_PANEL_WIDGET QObject::tr("viewpanel") //图片显示区域

#define PANEL_STACK QObject::tr("panelstack") //装有图片显示区域的视图窗口
#define MAIN_WIDGET QObject::tr("mainwidget") //主界面
#define MAIN_WIDOW QObject::tr("mainwindow") //主窗口
#define CENTER_WIDGET QObject::tr("centerwidget") //中心界面

#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
