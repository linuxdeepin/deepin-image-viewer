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
#ifndef SLIDESHOWPREVIEW_H
#define SLIDESHOWPREVIEW_H

#include <QFrame>

class SlideshowPreview : public QFrame
{
    Q_OBJECT
public:
    enum SlideshowEffect {
        Blinds,
        Switcher,
        Slide,
        Circle
    };

    explicit SlideshowPreview(SlideshowEffect effect, QWidget *parent = 0);
    void resetValue();

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    int activedEffectCount() const;
    bool checked() const;
    bool defaultValue() const;
    void setChecked(bool checked);

private:
    SlideshowEffect m_effect;
    int m_currentFrame;
    int m_animationTID;
    bool m_checked;
};

#endif // SLIDESHOWPREVIEW_H
