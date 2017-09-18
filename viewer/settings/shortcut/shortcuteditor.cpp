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
#include "shortcuteditor.h"
#include "application.h"
#include "controller/configsetter.h"
#include <QFocusEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QTextCodec>
#include <QDebug>

namespace {

const int MAX_WIDTH = 238;
const int MAX_HEIGHT = 22 + 2*2;
const int BORDER_RADIUS = 3;
const int ACTIVE_BORDER_WIDTH = 1;
const int NORMAL_BORDER_WIDTH = 1;
const QColor ACTIVE_BORDER_COLOR = QColor("#2ca7f8");
const QColor NORMAL_BORDER_COLOR = QColor(0, 0, 0, 255*0.08);

}

ShortcutEditor::ShortcutEditor(const QString &group, const QString &key, QWidget *parent)
    : QWidget(parent)
    , m_borderWidth(NORMAL_BORDER_WIDTH)
    , m_canSet(false)
    , m_borderColor(NORMAL_BORDER_COLOR)
    , m_group(group)
    , m_key(key)
{
    setFixedSize(MAX_WIDTH, MAX_HEIGHT);
    setFocusPolicy(Qt::ClickFocus);

    m_shortcut = defaultValue();
}

void ShortcutEditor::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);

    painter.save();
    QPainterPath bp;
    bp.addRoundedRect(rect(), BORDER_RADIUS, BORDER_RADIUS);
    painter.setClipPath(bp);

    // Draw inside border
    painter.setPen(QPen(m_borderColor, m_borderWidth));
    QPainterPathStroker stroker;
    stroker.setWidth(m_borderWidth);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath borderPath = stroker.createStroke(bp);
    painter.drawPath(borderPath);
    painter.restore();

    // Draw key
    // "Return" and "Enter" should show as "Enter" only
    QString s = m_shortcut.replace("Return", "Enter");
    QStringList keys = s.split("+", QString::SkipEmptyParts);

    if (keys.isEmpty()) {
        // Draw tips
        QRect tR(6, 5, width(), height());
        QFont f;
        f.setPixelSize(11);
        painter.setFont(f);
        painter.setPen(QPen(QColor(48, 48, 48, 0.4 * 255)));
        painter.drawText(tR, tr("Please enter a new shortcut"));
    }
    else {
        QRect lastRect(1, 0, 0, 0);
        for (QString key: keys) {
            painter.save();
            lastRect = drawTextRect(lastRect, key, painter);
            painter.restore();
        }
    }
}

void ShortcutEditor::focusInEvent(QFocusEvent *e)
{
    Q_UNUSED(e)

    m_borderWidth = ACTIVE_BORDER_WIDTH;
    m_borderColor = ACTIVE_BORDER_COLOR;
    this->update();
}

void ShortcutEditor::focusOutEvent(QFocusEvent *e)
{
    Q_UNUSED(e)

    m_borderWidth = NORMAL_BORDER_WIDTH;
    m_borderColor = NORMAL_BORDER_COLOR;
    m_shortcut = m_shortcut.isEmpty() ? defaultValue() : m_shortcut;
    this->update();
}

bool isModifiersKey(int key)
{
    QList<int> keys;
    keys << Qt::Key_Shift;
    keys << Qt::Key_Control;
    keys << Qt::Key_Alt;
    keys << Qt::Key_Meta;

    return keys.contains(key);
}

void ShortcutEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->key() != Qt::Key_Backspace) {
        if (isModifiersKey(e->key()) /*|| ! m_canSet*/)
            return;
        m_canSet = false;
        if (e->key() == Qt::Key_Delete) {
            setShortcut(QKeySequence(e->modifiers()).toString() + "Delete");
        }
        else {
            setShortcut(QKeySequence(e->modifiers()).toString()
                        + QKeySequence(e->key()).toString());
        }
    }
    else {
        m_canSet = true;
        m_shortcut = QString();
    }
    this->update();
}

void ShortcutEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    QWidget::mouseDoubleClickEvent(e);
    m_shortcut = QString();
    this->update();
}

void ShortcutEditor::forceUpdate()
{
    m_shortcut = defaultValue();
    this->update();
}

QSize ShortcutEditor::stringSize(const QString &str)
{
    QFont f;
    f.setPixelSize(10);
    QFontMetrics fm(f);
    int w = fm.boundingRect(str).width();
    int h = fm.height();

    return QSize(w, h);
}

QRect ShortcutEditor::drawTextRect(const QRect &lastRect, const QString &str, QPainter &painter)
{
    const QSize ss = stringSize(str);
    QRect r(lastRect.x() + lastRect.width() + 5, 2+2,
            ss.width() + 6*2, 18);

    QPainterPath bp;
    bp.addRoundedRect(r, BORDER_RADIUS, BORDER_RADIUS);
    painter.setClipPath(bp);

    painter.fillRect(r, QColor(105, 170, 255, 255*0.15));

    // Draw inside border
    painter.setPen(QPen(QColor(95, 159, 217, 255*0.3), 1));
    QPainterPathStroker stroker;
    stroker.setWidth(1);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath borderPath = stroker.createStroke(bp);
    painter.drawPath(borderPath);

    // Draw text
    QRect tR(r.x() + 6, 4, r.width(), r.height());
    QFont f;
    f.setPixelSize(11);
    painter.setFont(f);
    painter.setPen(QPen(QColor("#434343")));
    painter.drawText(tR, str);

    return r;
}

QString ShortcutEditor::defaultValue()
{
    return dApp->setter->value(m_group, m_key).toString();
}

void ShortcutEditor::updateValue()
{
    dApp->setter->setValue(m_group, m_key, m_shortcut);
}

void ShortcutEditor::setShortcut(const QString &shortcut)
{
    QStringList keys = dApp->setter->keys(m_group);
    for (QString key : keys) {
        if (dApp->setter->value(m_group, key).toString() == shortcut) {
            m_shortcut = defaultValue();
            return;
        }
    }

    m_shortcut = shortcut;
    updateValue();
}
