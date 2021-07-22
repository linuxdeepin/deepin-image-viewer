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
#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H


#include <QWidget>

class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);
    void setImage(const QImage &img);
    void setRectInImage(const QRect &r);
    void setAlwaysHidden(bool value);
    bool isAlwaysHidden() const;
    bool checkbgisdark(QImage &img) const;
    QPoint transImagePos(QPoint pos);

Q_SIGNALS:
    void requestMove(int x, int y);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    void tryMoveRect(const QPoint &p);
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    bool m_hide = false;
    qreal m_imageScale = 1.0;

    qreal m_widthScale {1.0};
    qreal m_heightScale {1.0};

    QImage m_img;
    QPixmap m_pix;
    QRectF m_r;          // Image visible rect
    QRect m_mainRect;
    QRect m_originRect;

    QRect imageDrawRect;

    QString m_bgImgUrl;
    QColor m_BgColor;
    QColor m_mrBgColor;
    QColor m_mrBorderColor;
    QColor m_imgRBorderColor;
};

#endif // NAVIGATIONWIDGET_H
