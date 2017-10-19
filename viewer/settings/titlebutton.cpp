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
#include "titlebutton.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace {

const int TITLE_HEIGHT = 30;
const int RIGHT_WIDTH = 3;
const int TITLE_LM_BIG = 34;
const int TITLE_LM_NORMAL = 44;
const int FONT_SIZE_BIG = 16;
const int FONT_SIZE_NORMAL = 12;
const QColor FONT_COLOR_ACTIVED = QColor("#2ca7f8");
const QColor FONT_COLOR_NORMAL = QColor("#303030");
const QColor LEFT_COLOR_ACTIVED = QColor(43, 167, 248, 50);
const QColor RIGHT_COLOR_ACTIVED = QColor("#2ca7f8");

}

TitleButton::TitleButton(SettingID id, bool bigFont, const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_bigFont(bigFont)
    , m_isActived(false)
    , m_id(id)
    , m_title(title)
{
    setFixedHeight(TITLE_HEIGHT);
}

void TitleButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit clicked(m_id);
        e->accept();
    }
}

void TitleButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    const QColor lbc = isActived() ? LEFT_COLOR_ACTIVED : QColor("#FFFFFF");
    const QColor rbc = isActived() ? RIGHT_COLOR_ACTIVED : QColor("#FFFFFF");
    const QRect lr(0, 0, width() - RIGHT_WIDTH, height());
    const QRect rr(width() - RIGHT_WIDTH, 0, RIGHT_WIDTH, height());

    const QRect bfr(TITLE_LM_BIG, (height() - FONT_SIZE_BIG) / 4,
                    width() - TITLE_LM_BIG, height());
    const QRect nfr(TITLE_LM_NORMAL, (height() - FONT_SIZE_NORMAL) / 3,
                    width() - TITLE_LM_NORMAL, height());

    // Background
    QPainter painter(this);
    painter.fillRect(lr, lbc);
    if (isActived()) {
        painter.fillRect(rr, rbc);

        // Active border
        QRect lbr(-1, 0, width() + 2, lr.height() - 1);
        QPainterPath path;
        path.addRect(lbr);
        painter.setPen(QPen(QColor(43, 167, 248, 25), 1));
        painter.drawPath(path);
    }

    // Title
    QFont f;
    if (m_bigFont)
        f.setWeight(QFont::Medium);
    f.setPixelSize(m_bigFont ? FONT_SIZE_BIG : FONT_SIZE_NORMAL);
    QPen p(isActived() ? FONT_COLOR_ACTIVED : FONT_COLOR_NORMAL);
    painter.setFont(f);
    painter.setPen(p);
    painter.drawText(m_bigFont ? bfr : nfr, m_title);
}

bool TitleButton::isActived() const
{
    return m_isActived;
}

void TitleButton::setIsActived(bool isActived)
{
    m_isActived = isActived;
    this->update();
}

TitleButton::SettingID TitleButton::id() const
{
    return m_id;
}

void TitleButton::setId(const SettingID &id)
{
    m_id = id;
}
