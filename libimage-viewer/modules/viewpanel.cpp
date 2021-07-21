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

ViewPanel::ViewPanel(QWidget *parent)
    : QFrame(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_stack = new DStackedWidget(this);
    layout->addWidget(m_stack, 0, Qt::AlignCenter);
    m_view = new ImageView(this);
    m_stack->addWidget(m_view);
}

ViewPanel::~ViewPanel()
{

}

void ViewPanel::loadImage(const QString &path)
{
    if (m_view) {
        m_view->setImage(path);
    }
}
