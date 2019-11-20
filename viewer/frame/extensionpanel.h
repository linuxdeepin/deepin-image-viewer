/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#include <QHBoxLayout>
#include <QPropertyAnimation>
#include "widgets/blureframe.h"
#include "controller/viewerthememanager.h"
#include <DFloatingWidget>

class ExtensionPanel : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit ExtensionPanel(QWidget *parent);
    void setContent(QWidget *content);
    void updateRectWithContent();
    void moveWithAnimation(int x, int y);
signals:
    void requestStopAnimation();
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
public slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    QColor m_coverBrush;
    QWidget *m_content;
    QHBoxLayout *m_contentLayout;
};

#endif // EXTENSIONPANEL_H
