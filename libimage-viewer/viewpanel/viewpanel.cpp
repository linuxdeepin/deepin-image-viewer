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
#include "viewpanel.h"

#include <QVBoxLayout>

#include "contents/bottomtoolbar.h"

const int BOTTOM_TOOLBAR_HEIGHT = 80;   //底部工具看高
const int BOTTOM_SPACING = 10;          //底部工具栏与底部边缘距离

ViewPanel::ViewPanel(QWidget *parent)
    : QFrame(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    m_stack = new DStackedWidget(this);
    layout->addWidget(m_stack, 0, Qt::AlignCenter);

    m_view = new ImageView(this);
    m_stack->addWidget(m_view);

    m_bottomToolbar = new BottomToolbar(this);
}

ViewPanel::~ViewPanel()
{

}

void ViewPanel::loadImage(const QString &path)
{
    if (m_view) {
        m_view->setImage(path);
        m_view->resetTransform();
        m_stack->setCurrentWidget(m_view);
        resetBottomToolbarGeometry(true);
    }
}

void ViewPanel::resetBottomToolbarGeometry(bool visible)
{
    m_bottomToolbar->setVisible(visible);
    if (visible) {
        int width = m_bottomToolbar->width();
        int x = (this->width() - m_bottomToolbar->width()) / 2;
        int y = this->height() - BOTTOM_TOOLBAR_HEIGHT - BOTTOM_SPACING;
        m_bottomToolbar->setGeometry(x, y, width, BOTTOM_TOOLBAR_HEIGHT);
    }
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    resetBottomToolbarGeometry(m_stack->currentWidget() == m_view);
    QFrame::resizeEvent(e);
}
