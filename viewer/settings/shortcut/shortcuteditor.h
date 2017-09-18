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
#ifndef SHORTCUTEDITOR_H
#define SHORTCUTEDITOR_H

#include <QWidget>

class ShortcutEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutEditor(const QString &group, const QString &key, QWidget *parent = 0);
    void forceUpdate();

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *e) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    QSize stringSize(const QString &str);
    QRect drawTextRect(const QRect &lastRect, const QString &str, QPainter &painter);
    QString defaultValue();
    void updateValue();
    void setShortcut(const QString &shortcut);

private:
    int m_borderWidth;
    bool m_canSet;

    QColor m_borderColor;
    QString m_group;
    QString m_key;
    QString m_shortcut;
};

#endif // SHORTCUTEDITOR_H
