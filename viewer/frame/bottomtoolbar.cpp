/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#include "bottomtoolbar.h"
#include "application.h"

namespace {
const QColor DARK_COVERCOLOR = QColor(26, 26, 26, 204);
const QColor LIGHT_COVERCOLOR = QColor(255, 255, 255, 230);
}

BottomToolbar::BottomToolbar(QWidget *parent)
    : BlurFrame(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &BottomToolbar::onThemeChanged);
}

void BottomToolbar::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_coverBrush = DARK_COVERCOLOR;
    } else {
        m_coverBrush = LIGHT_COVERCOLOR;
    }
    setCoverBrush(m_coverBrush);
}
void BottomToolbar::setContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_mainLayout->addWidget(content);
}

void BottomToolbar::mouseMoveEvent(QMouseEvent *)
{

}
