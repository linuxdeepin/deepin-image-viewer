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

#ifndef BOTTOMTOOLBAR_H
#define BOTTOMTOOLBAR_H

#include "widgets/blureframe.h"
#include "controller/viewerthememanager.h"

#include <QHBoxLayout>


class BottomToolbar : public BlurFrame
{
    Q_OBJECT
public:
    explicit BottomToolbar(QWidget *parent);
    void setContent(QWidget *content);

protected:
    void mouseMoveEvent(QMouseEvent *) override;

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QColor m_coverBrush;
    QHBoxLayout *m_mainLayout;

};

#endif // BOTTOMTOOLBAR_H
