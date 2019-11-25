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
#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QFutureWatcher>
#include "controller/viewerthememanager.h"

QT_BEGIN_NAMESPACE
class QWheelEvent;
class QPaintEvent;
class QFile;
class GraphicsMovieItem;
class GraphicsPixmapItem;
class QGraphicsSvgItem;
class QThreadPool;
class QGestureEvent;
class QPinchGesture;
class QSwipeGesture;
QT_END_NAMESPACE

#include "dtkwidget_global.h"
DWIDGET_BEGIN_NAMESPACE
class Toast;
DWIDGET_END_NAMESPACE

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL };

    ImageView(QWidget *parent = 0);

    void clear();
    void fitWindow();
    void fitImage();
    void rotateClockWise();
    void rotateCounterclockwise();
    void centerOn(int x, int y);
    void setImage(const QString &path);
    void setRenderer(RendererType type = Native);
    void setScaleValue(qreal v);

    void autoFit();

    const QImage image();
    qreal imageRelativeScale() const;
    qreal windowRelativeScale() const;
    const QRectF imageRect() const;
    const QString path() const;

    QPoint mapToImage(const QPoint &p) const;
    QRect mapToImage(const QRect &r) const;
    QRect visibleImageRect() const;
    bool isWholeImageVisible() const;

    bool isFitImage() const;
    bool isFitWindow() const;

signals:
    void clicked();
    void doubleClicked();
    void imageChanged(QString path);
    void mouseHoverMoved();
    void scaled(qreal perc);
    void transformChanged();
    void showScaleLabel();
    void hideNavigation();
    void nextRequested();
    void previousRequested();

public slots:
    void setHighQualityAntialiasing(bool highQualityAntialiasing);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    bool event(QEvent *event) override;

private slots:
    void onCacheFinish();
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

    void scaleAtPoint(QPoint pos, qreal factor);

    void handleGestureEvent(QGestureEvent *gesture);
    void pinchTriggered(QPinchGesture *gesture);
    void swipeTriggered(QSwipeGesture *gesture);

private:
    bool m_isFitImage;
    bool m_isFitWindow;
    QColor m_backgroundColor;
    RendererType m_renderer;
    QFutureWatcher<QVariantList> m_watcher;
    QString m_path;
    QString m_loadingIconPath;
    QThreadPool *m_pool;
    DTK_WIDGET_NAMESPACE::Toast *m_toast;

    QGraphicsSvgItem *m_svgItem = nullptr;
    GraphicsMovieItem *m_movieItem = nullptr;
    GraphicsPixmapItem *m_pixmapItem = nullptr;
};
#endif // SVGVIEW_H
