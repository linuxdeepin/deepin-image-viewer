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
#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include "controller/viewerthememanager.h"

#include <QWidget>
#include <QScrollArea>
#include <QFile>

class ThemeWidget : public QWidget
{
    Q_OBJECT
public:
    ThemeWidget(const QString &darkFile, const QString &lightFile,
                QWidget *parent = nullptr);
    ~ThemeWidget();

public slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
//    bool isDeepMode();
private:
    QString m_darkStyle;
    QString m_lightStyle;
    bool m_deepMode = false;
};

//TODO: if a widget Multiple Inheritance from ThemeWidget and
//      QScrollArea. warning(QWidget is an ambiguous base).
//class ThemeScrollArea : public QScrollArea {
//    Q_OBJECT
//public:
//    ThemeScrollArea(const QString &darkFile, const QString &lightFile,
//                QWidget* parent = 0);
//    ~ThemeScrollArea();

//public slots:
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);
//    bool isDeepMode();
//private:
//    QString m_darkStyle;
//    QString m_lightStyle;
//    bool m_deepMode = false;
//};
#endif // THEMEWIDGET_H
