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
#include "elidedlabel.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>

#include "application.h"
#include "controller/configsetter.h"

const int MAX_WIDTH = 600;
const int HEIGHT = 39;

ElidedLabel::ElidedLabel(QWidget *parent)
    : QLbtoDLabel(parent)
    , m_leftMargin(0)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &ElidedLabel::onThemeChanged);
}

ElidedLabel::~ElidedLabel()
{
}
void ElidedLabel::setText(const QString &text, int leftMargin)
{
    m_text = text;
    m_leftMargin = leftMargin;
    update();
}

void ElidedLabel::paintEvent(QPaintEvent *)
{
    QFont font = this->font();
    QFontMetrics fm(font);
    QPainter painter(this);
    QRect textR = QRect(m_leftMargin, 0, this->width() - m_leftMargin, this->height());
//    painter.fillRect(textR, QBrush(Qt::green));
    painter.setPen(QPen(m_textColor));
    painter.drawText(m_leftMargin, (this->height() - fm.height()) / 2,
                     this->width() - m_leftMargin, this->height(), Qt::AlignLeft, m_text);
    Q_UNUSED(textR);
}

void ElidedLabel::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    if (theme == ViewerThemeManager::Dark) {
        m_textColor = QColor(255, 255, 255, 204);
    } else {
        m_textColor = QColor("#656565");
    }
    update();
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    update();
}
