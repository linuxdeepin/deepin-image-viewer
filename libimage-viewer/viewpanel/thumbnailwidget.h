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
#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>

#include "widgets/themewidget.h"

#include <DLabel>

class QGestureEvent;
class QPinchGesture;
class QSwipeGesture;
class QPanGesture;

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class ThumbnailWidget : public ThemeWidget
{
    Q_OBJECT
public:
    ThumbnailWidget(const QString &darkFile, const QString
                    &lightFile, QWidget *parent = nullptr);
    ~ThumbnailWidget() override;
signals:
    void mouseHoverMoved();
    void nextRequested();
    void previousRequested();

    void sigMouseMove();
    void showfullScreen();
#ifdef LITE_DIV
    void openImageInDialog();
#endif

public slots:
    void handleGestureEvent(QGestureEvent *gesture);
    void setThumbnailImage(const QPixmap thumbnail);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

    bool event(QEvent *event) override;
private slots:
    void pinchTriggered(QPinchGesture *gesture);
private:
    void onThemeChanged(DGuiApplicationHelper::ColorType theme);

    bool m_isDefaultThumbnail = false;
    QLbtoDLabel *m_thumbnailLabel;
    QPixmap m_logo;
#ifndef LITE_DIV
    QLabel *m_tips;
#endif
    QPixmap m_defaultImage;
    QColor m_inBorderColor;
    QString m_picString;
    bool m_theme;
    bool m_usb = false;
    int m_startx = 0;
    int m_maxTouchPoints = 0;
};

#endif // THUMBNAILWIDGET_H
