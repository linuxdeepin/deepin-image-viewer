/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef DESKTOP_ACCESSIBLE_OBJECT_LIST_H
#define DESKTOP_ACCESSIBLE_OBJECT_LIST_H

#include "accessiblefunctions.h"
#include "ac-desktop-define.h"
#include "module/view/thumbnailwidget.h"
#include "module/view/lockwidget.h"

#include <QDialog>
#include <QPushButton>
#include <QListWidget>
#include <DTitlebar>
#include <QFrame>
#include <DMainWindow>
#include <QAction>
#include <QLabel>
#include <DSuggestButton>

DWIDGET_USE_NAMESPACE
// 添加accessible
//SET_FORM_ACCESSIBLE(CanvasGridView,OBJ_NAME_CANVAS_GRID_VIEW)
//SET_FORM_ACCESSIBLE(WaterMaskFrame,OBJ_NAME_WATER_MASK_FRAME)
SET_FORM_ACCESSIBLE(QFrame,m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget,m_w->objectName())
SET_LABEL_ACCESSIBLE(QLabel,m_w->objectName())
SET_FORM_ACCESSIBLE(QDialog,m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton,m_w->objectName())
SET_SLIDER_ACCESSIBLE(DMainWindow,m_w->objectName())
SET_SLIDER_ACCESSIBLE(QListWidget,m_w->objectName())
SET_FORM_ACCESSIBLE(DTitlebar,m_w->objectName())
//SET_FORM_ACCESSIBLE(Frame,m_w->objectName())
SET_BUTTON_ACCESSIBLE(DIconButton,m_w->objectName())
SET_FORM_ACCESSIBLE(QMenu,m_w->objectName())
SET_BUTTON_ACCESSIBLE(DSuggestButton,m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
//    USE_ACCESSIBLE(classname, CanvasGridView);
//    USE_ACCESSIBLE(classname, WaterMaskFrame);
    USE_ACCESSIBLE(classname, QFrame);
    USE_ACCESSIBLE(classname, QWidget);
    USE_ACCESSIBLE(classname, QLabel);
    USE_ACCESSIBLE(classname, QDialog);
    USE_ACCESSIBLE(classname, QPushButton);
    USE_ACCESSIBLE(classname, DMainWindow);
    USE_ACCESSIBLE(classname, QListWidget);
    USE_ACCESSIBLE(classname, DTitlebar);
//    USE_ACCESSIBLE(classname, Frame);
    USE_ACCESSIBLE(classname, QMenu);
    USE_ACCESSIBLE(classname, DIconButton);
    USE_ACCESSIBLE(classname, DSuggestButton);

    return interface;
}

#endif // DESKTOP_ACCESSIBLE_OBJECT_LIST_H
