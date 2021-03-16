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
#include "pushbutton.h"
#include "application.h"

#include <QDebug>
#include <QDesktopWidget>
#include <DFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QIcon>
#include <QApplication>

#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;
typedef DFrame QFrToDFrame;

PushButton::PushButton(QWidget *parent)
    : QWidget(parent)
    , m_checked(false)
    , m_isPressed(false)
    , m_spacing(2)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &PushButton::onThemeChanged);
}

QString PushButton::normalPic() const
{
    return m_normalPic;
}

QString PushButton::hoverPic() const
{
    return m_hoverPic;
}

QString PushButton::pressPic() const
{
    return m_pressPic;
}

QString PushButton::disablePic() const
{
    return m_disablePic;
}

QString PushButton::text() const
{
    return m_text;
}

QColor PushButton::normalColor() const
{
    return m_normalColor;
}

QColor PushButton::hoverColor() const
{
    return m_hoverColor;
}

QColor PushButton::pressColor() const
{
    return m_pressColor;
}

QColor PushButton::disableColor() const
{
    return m_disableColor;
}

void PushButton::setText(QString text)
{
    if (m_text == text)
        return;

    m_text = text;
    emit textChanged(text);
}

bool PushButton::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        if (QHelpEvent *he = static_cast<QHelpEvent *>(e)) {
            showTooltip(he->globalPos());

            return false;
        }
    }

    return QWidget::event(e);
}

void PushButton::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);

    QMargins m = contentsMargins();
    int ph = 0;
    int spacing = 0;

    qreal ration = this->devicePixelRatioF();
    QIcon icon(getPixmap());

    QPixmap pixmap = icon.pixmap(QPixmap(getPixmap()).size());
    pixmap.setDevicePixelRatio(ration);

    const qreal pixWidth = pixmap.width() / ration;
    const qreal pixHeight = pixmap.height() / ration;

    if (! pixmap.isNull()) {
        if (pixWidth > width() || pixHeight > height()) {
            ph = height() - m.top() - m.bottom();
            const QRect pr(m.left(), (height() - ph) / 2, ph, ph);
            painter.drawPixmap(QPoint(pr.x(), pr.y()), pixmap
                               /*pixmap.scaled(pr.size(), Qt::KeepAspectRatioByExpanding)*/);
        } else {
            ph = pixHeight;
            const QRect pr(m.left(), (height() - ph) / 2, pixWidth, ph);
            painter.drawPixmap(QPoint(pr.x(), pr.y()), pixmap);
        }
        spacing = m_spacing;
    }

    QFontMetrics fm(font());
    const int tw = width() - m.left() - spacing - ph - m.right();
    const int th = fm.height();
    const QRect textRect(m.left() + ph + spacing, (height() - th) / 2,
                         tw, th);
    const QString mt = fm.elidedText(m_text, Qt::ElideMiddle, tw);
    painter.setPen(QPen(getTextColor()));
    painter.drawText(textRect, Qt::AlignCenter, mt);
//    painter.setPen(Qt::red);
//    painter.drawRect(this->rect());
}

void PushButton::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    m_currentPic = hoverPic();
    m_currentColor = hoverColor();
    setCursor(Qt::PointingHandCursor);
    this->update();
}

void PushButton::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    m_currentColor = normalColor();
    m_currentPic = normalPic();
    setCursor(Qt::ArrowCursor);
    this->update();

    emit mouseLeave();
}

void PushButton::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    m_isPressed = true;
    m_currentColor = pressColor();
    m_currentPic = pressPic();
    this->update();
}

void PushButton::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    m_currentColor = normalColor();
    m_currentPic = normalPic();
    this->update();
    if (m_isPressed) {
        m_isPressed = false;
        emit clicked();
    }
}

QSize PushButton::sizeHint() const
{
    QPixmap p(getPixmap());
    QMargins m = contentsMargins();
    QFontMetrics fm(font());
    int spacing = p.isNull() ? 0 : m_spacing;
    int h = p.height() + m.top() + m.bottom();
    int w = p.width() + fm.width(m_text) + m.left() + m.right() + spacing + 3;

    return QSize(w, qMax(h, fm.height()));
}

QString PushButton::getPixmap() const
{
    if (m_checked) {
        return checkedPic();
    } else if (isEnabled()) {
        return m_currentPic;
    } else {
        return disablePic();
    }
}

QColor PushButton::getTextColor() const
{
    if (isEnabled()) {
        return m_currentColor;
    } else {
        return disableColor();
    }
}

void PushButton::showTooltip(const QPoint &pos)
{
    QFrToDFrame *tf = new QFrToDFrame();
//    tf->setStyleSheet(this->styleSheet());
    tf->setWindowFlags(Qt::ToolTip);
    tf->setAttribute(Qt::WA_TranslucentBackground);
    QLbtoDLabel *tl = new QLbtoDLabel(tf);
    tl->setObjectName("ButtonTooltip");
    tl->setText(toolTip());
    QHBoxLayout *layout = new QHBoxLayout(tf);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tl);

    tf->show();
    QRect dr = qApp->desktop()->geometry();
    int y = pos.y() + tf->height();
    if (y > dr.y() + dr.height()) {
        y = pos.y() - tf->height() - 10;
    }
    tf->move(pos.x() - tf->width() / 3, y - tf->height() / 3);

    QTimer::singleShot(5000, tf, SLOT(deleteLater()));

    connect(this, &PushButton::mouseLeave, tf, &QFrToDFrame::deleteLater);
    connect(this, &PushButton::clicked, tf, &QFrToDFrame::deleteLater);
}

void PushButton::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    Q_UNUSED(theme);
}

void PushButton::setChecked(bool checked)
{
    m_checked = checked;
    this->update();
}

QString PushButton::checkedPic() const
{
    return m_checkedPic;
}


