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
#include "themewidget.h"
#include "unionimage/baseutils.h"

//#include "application.h"
#include "accessibility/ac-desktop-define.h"

ThemeWidget::ThemeWidget(const QString &darkFile, const QString &lightFile,
                         QWidget *parent)
    : QWidget(parent)
{

    m_darkStyle = utils::base::getFileContent(darkFile);
    m_lightStyle = utils::base::getFileContent(lightFile);
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    onThemeChanged(themeType);
    setObjectName(THEME_WIDGET);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
                     this, &ThemeWidget::onThemeChanged);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ThemeWidget::onThemeChanged);
}

ThemeWidget::~ThemeWidget() {}

void ThemeWidget::onThemeChanged(DGuiApplicationHelper::ColorType theme)
{
    if (theme == DGuiApplicationHelper::ColorType::DarkType) {
        m_deepMode = true;
    } else {
        m_deepMode = false;
    }
}
