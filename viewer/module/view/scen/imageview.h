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
#include <QHash>
#include <QReadWriteLock>
#include "controller/viewerthememanager.h"

#include "imagesvgitem.h"

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

//#define PIXMAP_LOAD //用于判断是否采用pixmap加载，qimage加载会有内存泄露

class ImageView : public QGraphicsView
{
    Q_OBJECT

    //显示的图片类型枚举 add by heyi
    enum PICTURE_TYPE {
        NORMAL,         //普通图片
        SVG,            //SVG
        KINETOGRAM      //动态图片
    };

public:
    enum RendererType { Native, OpenGL };

    ImageView(QWidget *parent = nullptr);

    void clear();
    void fitWindow();
    void fitWindow_btnclicked();
    void fitImage();

    /**
     * @brief rotateClockWise   顺时针旋转90度
     */
    void rotateClockWise();

    /**
     * @brief rotateCounterclockwise 逆时针旋转90度
     */
    void rotateCounterclockwise();
    void centerOn(int x, int y);

    /**
     * @brief setImage  设置显示图片
     * @param path      显示的图片路径
     */
    void setImage(const QString path);

    QVariantList cachePixmap(const QString path);

    void setRenderer(RendererType type = Native);
    void setScaleValue(qreal v);

    void autoFit();
    void titleBarControl();

    const QImage image(bool brefresh = false);
    qreal imageRelativeScale() const;
    qreal windowRelativeScale() const;
    qreal windowRelativeScale_origin() const;
    const QRectF imageRect() const;

    /**
     * @brief path  当前显示图片路径
     * @return      图片路径
     */
    const QString path() const;

    QPoint mapToImage(const QPoint &p) const;
    QRect mapToImage(const QRect &r) const;
    QRect visibleImageRect() const;
    bool isWholeImageVisible() const;

    bool isFitImage() const;
    bool isFitWindow() const;

    /**
     * @brief rotatePixCurrent  判断当前图片是否被旋转，如果是，写入本地
     */
    void rotatePixCurrent();

    /**
     * @brief cacheThread   缓存图片线程，将缩略图的图片缓存到
     * @param strPath       需要缓存的图片路径
     */
    void cacheThread(const QString strPath);

    /**
     * @brief showPixmap    从hash中获取图片并显示
     * @param strPath       显示的图片路径
     */
    void showPixmap(QString strPath);

    /**
     * @brief judgePictureType  判断当前图片类型
     * @param strPath           图片路径
     * @return                  图片类型枚举
     */
    PICTURE_TYPE judgePictureType(const QString strPath);

    /**
     * @brief loadPictureByType 根据图片类型用不同的方式加载显示
     * @param type              图片类型
     * @param strPath           图片路径
     * @return                  true为加载成功，false为加载失败
     */
    bool loadPictureByType(PICTURE_TYPE type, const QString strPath);

signals:
    void clicked();
    void doubleClicked();
    void imageChanged(QString path);
    void mouseHoverMoved();
    void scaled(qreal perc);
    void transformChanged();
    void showScaleLabel();
//    void hideNavigation();
    void nextRequested();
    void previousRequested();
    void disCheckAdaptImageBtn();
    void checkAdaptImageBtn();

    /**
     * @brief cacheEnd  当前显示图片缓存
     */
    void cacheEnd();

    /**
     * @brief cacheThreadEnd
     * @param vl
     */
    void cacheThreadEndSig(QVariantList vl);

public slots:
    void setHighQualityAntialiasing(bool highQualityAntialiasing);

    /**
     * @brief endApp    结束程序触发此槽函数
     */
    void endApp();

    /**
     * @brief reloadSvgPix  重新加载svg图片
     * @param strPath       图片路径
     * @param nAngel        旋转角度
     * @return              true为加载成功，false为加载失败
     */
    bool reloadSvgPix(QString strPath, int nAngel);

    /**
     * @brief rotatePixmap  根据角度旋转pixmap
     * @param nAngel        旋转的角度
     */
    void rotatePixmap(int nAngel);

    /**
     * @brief recvPathsToCache  接收图片路径进行缓存
     * @param pathsList         需要缓存的图片路径
     */
    void recvPathsToCache(const QStringList pathsList);

    /**
     * @brief delCacheFromPath  根据图片路径删除缓存
     * @param strPath           删除的图片路径
     */
    void delCacheFromPath(const QString strPath);

    /**
     * @brief delAllCache   删除所有缓存
     */
    void delAllCache();

    /**
     * @brief removeDiff    判断两次图片路径差异，将差异部分缓存删除并缓存新的图片
     * @param pathsList     传入的需要缓存的图片
     * @return
     */
    QStringList removeDiff(QStringList pathsList);

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
    /**
     * @brief onCacheFinish 普通图片缓存结束
     */
    void onCacheFinish(QVariantList vl);

    /**
     * @brief onThemeChanged 主题切换
     * @param theme          切换的主题
     */
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

    /**
     * @brief scaleAtPoint  在指定位置缩放
     * @param pos           鼠标位置
     * @param factor        缩放大小
     */
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

    ImageSvgItem *m_imgSvgItem {nullptr};

    GraphicsMovieItem *m_movieItem = nullptr;
    GraphicsPixmapItem *m_pixmapItem = nullptr;
    //缓存锁
    QReadWriteLock m_rwCacheLock;
    QHash<QString, QPixmap> m_hsPixap;
//    QHash<QString, QSvgRenderer> m_hsSvg;
//    QHash<QString, GraphicsMovieItem> m_hsMovie;
    QStringList m_pathsList;
    QStringList m_pLastPaths;

    bool m_loadingDisplay = false;
    //heyi test 保存旋转的角度
    int m_rotateAngel = 0;
    QImage m_svgimg;
};
#endif // SVGVIEW_H
