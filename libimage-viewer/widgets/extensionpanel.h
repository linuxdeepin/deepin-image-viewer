/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXTENSIONPANEL_H
#define EXTENSIONPANEL_H

#include <DDialog>
#include <DFloatingWidget>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <DTitlebar>
#include <QShortcut>

//#include "controller/viewerthememanager.h"
#include "widgets/blureframe.h"
#include <DWidget>
DWIDGET_USE_NAMESPACE
typedef DWidget QWdToDWidget;

// class ExtensionPanel : public DFloatingWidget
class ExtensionPanel : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit ExtensionPanel(QWidget *parent = nullptr);
    void setContent(QWidget *content);

//    void moveWithAnimation(int x, int y);

    void init();
signals:
    void requestStopAnimation();
public slots:
    void updateRectWithContent(int height);
private:
    QColor m_coverBrush;
    QWdToDWidget *m_content;
    QVBoxLayout *m_contentLayout;

    QVBoxLayout *m_mainLayout {nullptr};
    QScrollArea *m_scrollArea {nullptr};

    Dtk::Widget::DTitlebar *m_titleBar {nullptr};

    QShortcut *m_scImageInfo{nullptr};
    QShortcut *m_scImageInfonum{nullptr};
    QShortcut *m_scEsc {nullptr};
};

#endif  // EXTENSIONPANEL_H
