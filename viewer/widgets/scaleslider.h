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
#ifndef SCALESLIDER_H
#define SCALESLIDER_H

#include <QSlider>

class ScaleSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor)
public:
    enum SliderShape {
        Line,
        Rect,
    };
    explicit ScaleSlider(SliderShape defaultShape = SliderShape::Line, QWidget *parent = 0);

    QColor penColor() const;
    QColor brushColor() const;
    void setPenColor(const QColor &penColor);
    void setBrushColor(const QColor &brushColor);
signals:
    void mousePress();
    void mouseRelease();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    SliderShape m_defaultShape = SliderShape::Line;

    void paintEvent(QPaintEvent *e) override;
    QColor m_penColor;
    QColor m_brushColor;
};

#endif // SCALESLIDER_H
