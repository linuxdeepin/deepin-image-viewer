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
#include "title.h"
#include <QFontMetrics>
#include <QPainter>

namespace {

const int ITEM_SPACING = 15;
const int LEFT_MARGIN = 21;
const int RIGHT_MARGIN = 14;
const int SEPARATOR_SIZE = 1;
const int FONT_SIZE = 14;
const QColor SEPARATOR_COLOR = QColor(0, 0, 0, 25);
const QColor FONT_COLOR = QColor("#303030");

}

Title1::Title1(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_title(title)
{
    setFixedHeight(20);
}

void Title1::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QFont f;
    f.setWeight(QFont::DemiBold);
    f.setPixelSize(FONT_SIZE);
    const int tw = QFontMetrics(f).width(m_title);
    const int th = QFontMetrics(f).height();

    const QRect tr(LEFT_MARGIN, 0, tw, th);
    const int sx = tr.x() + tr.width() + ITEM_SPACING;
    const QRect sr(sx, (height() - SEPARATOR_SIZE) / 2,
                   width() - sx - RIGHT_MARGIN, SEPARATOR_SIZE);

    QPainter painter(this);
    painter.setFont(f);
    painter.setPen(QPen(FONT_COLOR));
    painter.drawText(tr, m_title);

    painter.fillRect(sr, SEPARATOR_COLOR);
}

Title2::Title2(const QString &title, QWidget *parent)
    :QLabel(title, parent)
{

}

Title3::Title3(const QString &title, QWidget *parent)
    :QLabel(title, parent)
{

}
