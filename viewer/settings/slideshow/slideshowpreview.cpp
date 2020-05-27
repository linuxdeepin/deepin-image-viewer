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
#include "slideshowpreview.h"
#include "application.h"
#include "controller/configsetter.h"
#include <QFontMetrics>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QTimerEvent>

namespace {

const int FONT_SIZE = 10;
const int TITLE_HEIGHT = 15;
const int CHECK_ICON_SIZE = 16;
const int PIXMAP_WIDTH = 96;
const int PIXMAP_HEIGHT = 60;
const int MAX_WIDTH = PIXMAP_WIDTH + CHECK_ICON_SIZE;
const int MAX_HEIGHT = PIXMAP_HEIGHT + CHECK_ICON_SIZE / 2 + 4 + TITLE_HEIGHT;

const QString SETTING_GROUP = "SLIDESHOWEFFECT";

QMap<int, QString> nameMap()
{
    QMap<int, QString> nm;
    nm.insert(SlideshowPreview::Blinds, QObject::tr("Blinds"));
    nm.insert(SlideshowPreview::Switcher, QObject::tr("Switcher"));
    nm.insert(SlideshowPreview::Slide, QObject::tr("Sliding"));
    nm.insert(SlideshowPreview::Circle, QObject::tr("Ring"));

    return nm;
}

const QString PREVIEW_ROOT_PATH = ":/settings/images/slideshow/assets/images/slideshow/";
QMap<int, QString> pathMap()
{
    QMap<int, QString> pm;
    pm.insert(SlideshowPreview::Blinds, PREVIEW_ROOT_PATH + "Blinds/");
    pm.insert(SlideshowPreview::Switcher, PREVIEW_ROOT_PATH + "Switcher/");
    pm.insert(SlideshowPreview::Slide, PREVIEW_ROOT_PATH + "Slide/");
    pm.insert(SlideshowPreview::Circle, PREVIEW_ROOT_PATH + "Circle/");

    return pm;
}

}

SlideshowPreview::SlideshowPreview(SlideshowEffect effect, QWidget *parent)
    : QFrToDFrame(parent)
    , m_effect(effect)
    , m_currentFrame(0)
{
    setFixedSize(MAX_WIDTH, MAX_HEIGHT);

    m_checked = defaultValue();
}

void SlideshowPreview::resetValue()
{
    dApp->setter->setValue(SETTING_GROUP, QString::number(m_effect), true);
    m_checked = true;
    this->update();
}

void SlideshowPreview::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw frame
    const QString pp = pathMap().value(m_effect) +
            QString::number(m_currentFrame) + ".png";
    QRect pixmapRect(CHECK_ICON_SIZE / 2, CHECK_ICON_SIZE / 2,
                     PIXMAP_WIDTH, PIXMAP_HEIGHT);
    painter.drawPixmap(pixmapRect, QPixmap(pp));

    // Draw title
    const QString name = nameMap().value(m_effect);
    QFont f;
    f.setPixelSize(FONT_SIZE);
    const int tw = QFontMetrics(f).width(name);
    QRect titleRect((MAX_WIDTH - tw) / 2, pixmapRect.y() + pixmapRect.height() + 4,
                    tw, 15);
    painter.setPen(QPen(QColor("#303030")));
    painter.setFont(f);
    painter.drawText(titleRect, name);

    // Draw check state
    const QString cip = checked()
            ? ":/settings/images/assets/images/checkbox_checked.png"
            : ":/settings/images/assets/images/checkbox_unchecked.png";
    QRect checkRect(MAX_WIDTH - CHECK_ICON_SIZE, 0,
                    CHECK_ICON_SIZE, CHECK_ICON_SIZE);
    painter.drawPixmap(checkRect, QPixmap(cip));
}

void SlideshowPreview::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_animationTID) {
        if (m_currentFrame == 55) {
            m_currentFrame = 0;
        }
        else {
            m_currentFrame ++;
        }

        this->update();
    }

    QFrToDFrame::timerEvent(e);
}

void SlideshowPreview::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    m_animationTID = startTimer(1000 / 25);
}

void SlideshowPreview::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    killTimer(m_animationTID);
    m_animationTID = 0;
    m_currentFrame = 0;
    this->update();
}

void SlideshowPreview::mousePressEvent(QMouseEvent *e)
{
    // One effect at least
    if (activedEffectCount() <= 1 && m_checked) {
        QFrToDFrame::mousePressEvent(e);
    }
    else if (e->button() == Qt::LeftButton) {
        e->accept();
        dApp->setter->setValue(SETTING_GROUP, QString::number(m_effect),
                               QVariant(! m_checked));
        m_checked = ! m_checked;
        this->update();
    }
}

int SlideshowPreview::activedEffectCount() const
{
    int count = 0;
    QStringList keys = dApp->setter->keys(SETTING_GROUP);
    for (QString key : keys) {
        if (dApp->setter->value(SETTING_GROUP, key).toBool()) {
            count ++;
        }
    }
    return count;
}

bool SlideshowPreview::checked() const
{
    return m_checked;
}

bool SlideshowPreview::defaultValue() const
{
    QVariant v = dApp->setter->value(SETTING_GROUP, QString::number(m_effect));
    if (v.isNull()) {
        dApp->setter->setValue(SETTING_GROUP, QString::number(m_effect), true);
        return true;
    }
    else {
        return v.toBool();
    }
}

void SlideshowPreview::setChecked(bool checked)
{
    m_checked = checked;
}
