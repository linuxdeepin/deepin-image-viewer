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
#include "viewerthememanager.h"
#include "application.h"
#include "configsetter.h"

namespace {
const QString THEME_GROUP = "APP";
const QString THEME_TEXT = "AppTheme";
}

ViewerThemeManager *ViewerThemeManager::m_viewerTheme = NULL;
ViewerThemeManager *ViewerThemeManager::instance()
{
    if (m_viewerTheme == NULL) {
        m_viewerTheme = new ViewerThemeManager;
    }

    return m_viewerTheme;
}

ViewerThemeManager::ViewerThemeManager(QObject *parent) : QObject(parent)
{
}

ViewerThemeManager::AppTheme ViewerThemeManager::getCurrentTheme()
{
    return m_currentTheme;
}

void ViewerThemeManager::setCurrentTheme(AppTheme theme)
{
    m_currentTheme = theme;
    if (m_currentTheme == Dark)
        dApp->setter->setValue(THEME_GROUP, THEME_TEXT, QVariant("Dark"));
    else
        dApp->setter->setValue(THEME_GROUP, THEME_TEXT, QVariant("Light"));
    emit viewerThemeChanged(m_currentTheme);
}
