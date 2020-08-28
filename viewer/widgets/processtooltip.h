/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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
#ifndef PROCESSTOOLTIP_H
#define PROCESSTOOLTIP_H

#include "blureframe.h"
#include "application.h"
#include "controller/viewerthememanager.h"

#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class ProcessTooltip : public BlurFrame
{
    Q_OBJECT
public:
    explicit ProcessTooltip(QWidget *parent);
    void showTooltip(const QString &message, bool success);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QLbtoDLabel *m_icon;
    QLbtoDLabel *m_message;

    QColor m_coverColor;
    QColor m_borderColor;
    QString m_textColor;
};

#endif // PROCESSTOOLTIP_H
