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
#include "themewidget.h"
#include "utils/baseutils.h"

#include "application.h"

ThemeWidget::ThemeWidget(const QString &darkFile, const QString &lightFile,
                         QWidget *parent)
    : QWidget(parent) {

    m_darkStyle = utils::base::getFileContent(darkFile);
    m_lightStyle = utils::base::getFileContent(lightFile);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThemeWidget::onThemeChanged);
}

bool ThemeWidget::isDeepMode() {
    return m_deepMode;
}

ThemeWidget::~ThemeWidget() {}

void ThemeWidget::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
//        this->setStyleSheet(m_darkStyle);
        m_deepMode = true;
    } else {
//        this->setStyleSheet(m_lightStyle);
        m_deepMode = false;
    }
}

ThemeScrollArea::ThemeScrollArea(const QString &darkFile, const QString &lightFile,
                         QWidget *parent)
    : QScrollArea(parent) {

    m_darkStyle = utils::base::getFileContent(darkFile);
    m_lightStyle = utils::base::getFileContent(lightFile);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThemeScrollArea::onThemeChanged);
}

ThemeScrollArea::~ThemeScrollArea() {}

void ThemeScrollArea::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
//        this->setStyleSheet(m_darkStyle);
        m_deepMode = true;
    } else {
//        this->setStyleSheet(m_lightStyle);
        m_deepMode = false;
    }
}

bool ThemeScrollArea::isDeepMode() {
    return m_deepMode;
}
