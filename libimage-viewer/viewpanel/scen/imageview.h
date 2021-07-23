/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
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
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QFutureWatcher>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QGraphicsBlurEffect>
#include <QPointer>
#include <QMap>
#include <QFileSystemWatcher>

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
DWIDGET_END_NAMESPACE

class CFileWatcher;
class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL };

    explicit ImageView(QWidget *parent = nullptr);
    ~ImageView() override;
    void clear();
    void fitWindow();
    void fitImage();
    void rotateClockWise();
    void rotateCounterclockwise();
    void centerOn(qreal x, qreal y);
    void setImage(const QString &path, const QImage &image = QImage());
//    void setRenderer(RendererType type = Native);
    void setScaleValue(qreal v);

    void autoFit();

    const QImage image();
    qreal imageRelativeScale() const;
    qreal windowRelativeScale() const;
//    const QRectF imageRect() const;
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
    void imageChanged(const QString &path);
    void mouseHoverMoved();
    void scaled(qreal perc);
    void transformChanged();
    void showScaleLabel();
    void hideNavigation();
    void nextRequested();
    void previousRequested();
    void disCheckAdaptImageBtn();
    void disCheckAdaptScreenBtn();
    void checkAdaptImageBtn();
    void checkAdaptScreenBtn();
    void sigFIleDelete();


public slots:
//    void setHighQualityAntialiasing(bool highQualityAntialiasing);
    void onImgFileChanged(const QString &ddfFile);
    void onLoadTimerTimeout();
    void onThemeTypeChanged();
    void onIsChangedTimerTimeout();



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
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    void scaleAtPoint(QPoint pos, qreal factor);
    void handleGestureEvent(QGestureEvent *gesture);
    void pinchTriggered(QPinchGesture *gesture);
//    void swipeTriggered(QSwipeGesture *gesture);
//    void updateImages(const QStringList &path);
private:
    bool m_isFitImage = false;
    bool m_isFitWindow = false;
    QColor m_backgroundColor;
    RendererType m_renderer;
    QFutureWatcher<QVariantList> m_watcher;
    QString m_path;
    QString m_loadingIconPath;
    QThreadPool *m_pool;
    GraphicsMovieItem *m_movieItem = nullptr;
    GraphicsPixmapItem *m_pixmapItem = nullptr;
    QPointer<QGraphicsBlurEffect> m_blurEffect;
//    CFileWatcher *m_imgFileWatcher;
    QFileSystemWatcher *m_imgFileWatcher;
    QTimer *m_isChangedTimer;

    bool m_isFirstPinch = false;
    QPointF m_centerPoint;
    QTimer *m_loadTimer = nullptr;
    QString m_loadPath;//需要加载的图片路径
    int m_startpointx = 0;//触摸操作放下时的x坐标
    int m_maxTouchPoints = 0;//触摸动作时手指数

    //平板需求，记录打开图片时初始缩放比例
    bool m_firstset = false;
    double m_value = 0.0;
    double m_max_scale_factor = 2.0;
    double m_min_scale_factor = 0.0;

    //单指点击标识位
    bool m_press = false;
};

//class CFileWatcher: public QThread
//{
//    Q_OBJECT
//public:
//    enum EFileChangedType {EFileModified, EFileMoved, EFileCount};

//    explicit CFileWatcher(QObject *parent = nullptr);
//    ~CFileWatcher();

//    bool isVaild();

//    void addWather(const QString &path);
////    void removePath(const QString &path);

//    void clear();

//signals:
//    void fileChanged(const QString &path, int tp);

//protected:
//    void run();

//private:
//    void doRun();

//    int  _handleId = -1;
//    bool _running = false;


//    QMap<QString, int> watchedFiles;
//    QMap<int, QString> watchedFilesId;

//    QMutex _mutex;
//};
#endif // IMAGEVIEW_H
