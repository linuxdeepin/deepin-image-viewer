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
#include "imagegraphicsview.h"

#include <QDebug>
#include <QFile>
#include <QOpenGLWidget>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QMovie>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QPaintEvent>
#include <QtConcurrent>
#include <QHBoxLayout>
#include <qmath.h>
#include <QScrollBar>
#include <QGestureEvent>
#include <QSvgRenderer>
#include <QtGlobal>
#include <QDesktopWidget>
#include <QApplication>

#include "graphicsitem.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "accessibility/ac-desktop-define.h"
#include <DGuiApplicationHelper>
#include <DApplicationHelper>

#ifndef QT_NO_OPENGL
//#include <QGLWidget>
#endif

#include <sys/inotify.h>
#include "service/commonservice.h"
DWIDGET_USE_NAMESPACE

namespace {

const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");

#ifndef tablet_PC
const qreal MAX_SCALE_FACTOR = 20.0;
const qreal MIN_SCALE_FACTOR = 0.02;
#else
const qreal MAX_SCALE_FACTOR = 2.0;
qreal MIN_SCALE_FACTOR = 0.0;
#endif

QVariantList cachePixmap(const QString &path)
{
    QImage tImg;
    QString errMsg;
    QSize size;
//    UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, size, errMsg);
    UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg);
    QPixmap p = QPixmap::fromImage(tImg);
    if (QFileInfo(path).exists() && p.isNull()) {
        //判定为损坏图片
//        p = utils::image::getDamagePixmap(DApplicationHelper::instance()->themeType() == DApplicationHelper::LightType);
        qDebug() << errMsg;
    }
    QVariantList vl;
    vl << QVariant(path) << QVariant(p);
    return vl;
}

}  // namespace
ImageGraphicsView::ImageGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_renderer(Native)
    , m_pool(new QThreadPool(this))
//    , m_svgItem(nullptr)
    , m_movieItem(nullptr)
    , m_pixmapItem(nullptr)
{
    this->setObjectName("ImageView");
//    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    setScene(new QGraphicsScene(this));
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);
    setAcceptDrops(false);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::Shape::NoFrame);

    viewport()->setCursor(Qt::ArrowCursor);

    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);

    connect(&m_watcher, &QFutureWatcherBase::finished, this, &ImageGraphicsView::onCacheFinish);
//    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this, &ImageView::onThemeChanged);
    m_pool->setMaxThreadCount(1);
    m_loadTimer = new QTimer(this);
    m_loadTimer->setSingleShot(true);
    m_loadTimer->setInterval(300);

    connect(m_loadTimer, &QTimer::timeout, this, &ImageGraphicsView::onLoadTimerTimeout);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ImageGraphicsView::onThemeTypeChanged);
    m_imgFileWatcher = new QFileSystemWatcher(this);
    connect(m_imgFileWatcher, &QFileSystemWatcher::fileChanged, this, &ImageGraphicsView::onImgFileChanged);
    m_isChangedTimer = new QTimer(this);
    QObject::connect(m_isChangedTimer, &QTimer::timeout, this, &ImageGraphicsView::onIsChangedTimerTimeout);
}

ImageGraphicsView::~ImageGraphicsView()
{
    if (m_imgFileWatcher) {
//        m_imgFileWatcher->clear();
//        m_imgFileWatcher->quit();
//        m_imgFileWatcher->terminate();
//        m_imgFileWatcher->wait();
        m_imgFileWatcher->deleteLater();
    }
}

void ImageGraphicsView::clear()
{
    if (m_pixmapItem != nullptr) {
        delete m_pixmapItem;
        m_pixmapItem = nullptr;
    }
    m_movieItem = nullptr;
    scene()->clear();
}

void ImageGraphicsView::setImage(const QString &path, const QImage &image)
{
    m_loadPath = path;
    // Empty path will cause crash in release-build mode
    if (path.isEmpty()) {
        return;
    }
    m_imgFileWatcher->removePath(m_path);
    m_path = path;
    m_imgFileWatcher->addPath(m_path);
    QString strfixL = QFileInfo(path).suffix().toUpper();
    QGraphicsScene *s = scene();
    QFileInfo fi(path);
    QStringList fList = UnionImage_NameSpace::supportMovieFormat(); //"gif","mng"
    //QMovie can't read frameCount of "mng" correctly,so change
    //the judge way to solve the problem
    if (fList.contains(strfixL)) {
        if (m_pixmapItem != nullptr) {
            delete m_pixmapItem;
            m_pixmapItem = nullptr;
        }
        s->clear();
        resetTransform();
        m_movieItem = new GraphicsMovieItem(path, strfixL);
        //        m_movieItem->start();
        // Make sure item show in center of view after reload
        setSceneRect(m_movieItem->boundingRect());
        qDebug() << "m_movieItem->boundingRect() = " << m_movieItem->boundingRect();
        s->addItem(m_movieItem);
        emit imageChanged(path);
        QMetaObject::invokeMethod(this, [ = ]() {
            resetTransform();
            autoFit();
        }, Qt::QueuedConnection);
    } else {
        //当传入的image无效时，需要重新读取数据
        m_movieItem = nullptr;
        qDebug() << "Start cache pixmap: " << path;
        if (image.isNull()) {
            QImageReader imagreader(path);      //取原图的分辨率
            int w = imagreader.size().width();
            int h = imagreader.size().height();
            scene()->clear();
            resetTransform();
            int wScale = 0;
            int hScale = 0;
            int wWindow = 0;
            int hWindow = 0;
            if (QApplication::activeWindow()) {
                wWindow = QApplication::activeWindow()->width();
                hWindow = QApplication::activeWindow()->height();
            } else {
                wWindow = 1300;
                hWindow = 848;
            }

            if (w >= wWindow) {
                wScale = wWindow;
                hScale = wScale * h / w;
                if (hScale > hWindow) {
                    hScale = hWindow;
                    wScale = hScale * w / h;
                }
            } else if (h >= hWindow) {
                hScale = hWindow;
                wScale = hScale * w / h;
                if (wScale >= wWindow) {
                    wScale = wWindow;
                    hScale = wScale * h / w;
                }
            } else {
                wScale = w;
                hScale = h;
            }
            if (wScale == 0 || wScale == -1) {
                wScale = wWindow;
                hScale = hWindow;
            }
            QPixmap pix;
//            QImage image = CommonService::instance()->getImgByPath(path);
//            if (image.isNull()) {
//                pix = QPixmap(path).scaled(wScale, hScale, Qt::KeepAspectRatio); //缩放到原图大小
//            } else {
//                pix = QPixmap::fromImage(image).scaled(wScale, hScale, Qt::KeepAspectRatio); //缩放到原图大小
//            }
            pix = QPixmap(path).scaled(wScale, hScale, Qt::KeepAspectRatio); //缩放到原图大小

//            QPixmap pix = QPixmap(path).scaled(wScale, hScale, Qt::KeepAspectRatio); //缩放到原图大小
            qDebug() << "---" << __FUNCTION__ << "-----------" << pix.width() << "----------" << pix.height();
            m_pixmapItem = new GraphicsPixmapItem(pix);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
            // Make sure item show in center of view after reload
            m_blurEffect = new QGraphicsBlurEffect;
            m_blurEffect->setBlurRadius(5);
            m_blurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
            m_pixmapItem->setGraphicsEffect(m_blurEffect);
            setSceneRect(m_pixmapItem->boundingRect());
            m_loadTimer->start();
            scene()->addItem(m_pixmapItem);
            emit imageChanged(path);
            QMetaObject::invokeMethod(this, [ = ]() {
                resetTransform();
            }, Qt::QueuedConnection);
        } else {
            //当传入的image有效时，直接刷入图像，不再重复读取
            QPixmap pix = QPixmap::fromImage(image);
            pix.setDevicePixelRatio(devicePixelRatioF());
            m_pixmapItem = new GraphicsPixmapItem(pix);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
            setSceneRect(m_pixmapItem->boundingRect());
            scene()->clear();
            scene()->addItem(m_pixmapItem);
            autoFit();
            emit imageChanged(path);
            this->update();
            emit hideNavigation();
        }
    }
    m_firstset = true;
}

void ImageGraphicsView::setScaleValue(qreal v)
{
    scale(v, v);
#ifdef tablet_PC
    //平板模式下限制缩放范围：初始值--200%
    if (m_firstset) {
        m_firstset = false;
        m_value = imageRelativeScale();
    }
    const qreal irs = imageRelativeScale();
    if ((v < 1 && irs <= m_value)) {
        const qreal minv = m_value / irs;
        scale(minv, minv);
    } else if (v > 1 && irs >= m_max_scale_factor) {
        const qreal maxv = m_max_scale_factor / irs;
        scale(maxv, maxv);
    } else {
        m_isFitImage = false;
        m_isFitWindow = false;
    }
#else
    const qreal irs = imageRelativeScale();
    // Rollback
    if ((v < 1 && irs <= MIN_SCALE_FACTOR)) {
        const qreal minv = MIN_SCALE_FACTOR / irs;
        scale(minv, minv);
    } else if (v > 1 && irs >= MAX_SCALE_FACTOR) {
        const qreal maxv = MAX_SCALE_FACTOR / irs;
        scale(maxv, maxv);
    } else {
        m_isFitImage = false;
        m_isFitWindow = false;
    }
#endif

    qreal rescale = imageRelativeScale();
    if (rescale - 1 > -0.01 &&
            rescale - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }

    qreal wrs = windowRelativeScale();
    if (rescale - wrs > -0.01 &&
            rescale - wrs < 0.01) {
        emit checkAdaptScreenBtn();
    } else {
        emit disCheckAdaptScreenBtn();
    }
    emit scaled(imageRelativeScale() * 100);
    emit showScaleLabel();
    emit transformChanged();
}

void ImageGraphicsView::autoFit()
{
    //确认场景加载出来后，才能调用场景内的item
//    if (!scene()->isActive())
//        return;
    if (image().isNull())
        return;
    QSize image_size = image().size();
    if ((image_size.width() >= width() ||
            image_size.height() >= height()) &&
            width() > 0 && height() > 0) {
        fitWindow();
    } else {
        fitImage();
    }
}

const QImage ImageGraphicsView::image()
{
    if (m_movieItem) {           // bit-map
        return m_movieItem->pixmap().toImage();
    } else if (m_pixmapItem) {
        //FIXME: access to m_pixmapItem will crash
        if (nullptr == m_pixmapItem) {  //add to slove crash by shui
            return QImage();
        }
        return m_pixmapItem->pixmap().toImage();
    } else {
        return QImage();
    }
}

void ImageGraphicsView::fitWindow()
{
    qreal wrs = windowRelativeScale();
    resetTransform();
    scale(wrs, wrs);
    emit checkAdaptScreenBtn();
    if (wrs - 1 > -0.01 &&
            wrs - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }
    m_isFitImage = false;
    m_isFitWindow = true;
    scaled(imageRelativeScale() * 100);
    emit transformChanged();
}

void ImageGraphicsView::fitImage()
{
    qreal wrs = windowRelativeScale();
    resetTransform();
    scale(1, 1);
    emit checkAdaptImageBtn();
    if (wrs - 1 > -0.01 &&
            wrs - 1 < 0.01) {
        emit checkAdaptScreenBtn();
    } else {
        emit disCheckAdaptScreenBtn();
    }
    m_isFitImage = true;
    m_isFitWindow = false;
    scaled(imageRelativeScale() * 100);
    emit transformChanged();
}

void ImageGraphicsView::rotateClockWise()
{
    QString errMsg;
    QImage rotateResult;
    if (!UnionImage_NameSpace::rotateImageFIleWithImage(90, rotateResult, m_path, errMsg)) {
        qDebug() << errMsg;
        return;
    }
//    dApp->m_imageloader->updateImageLoader(QStringList(m_path), QList<QImage>({rotateResult}));
    setImage(m_path, rotateResult);
}

void ImageGraphicsView::rotateCounterclockwise()
{
    QString errMsg;
    QImage rotateResult;
    if (!UnionImage_NameSpace::rotateImageFIleWithImage(-90, rotateResult, m_path, errMsg)) {
        qDebug() << errMsg;
        return;
    }
//    dApp->m_imageloader->updateImageLoader(QStringList(m_path), QList<QImage>({rotateResult}));
    setImage(m_path, rotateResult);
}

void ImageGraphicsView::centerOn(qreal x, qreal y)
{
    QGraphicsView::centerOn(x, y);
    emit transformChanged();
}

qreal ImageGraphicsView::imageRelativeScale() const
{
    // vertical scale factor are equal to the horizontal one
    return transform().m11();
}

qreal ImageGraphicsView::windowRelativeScale() const
{
    QRectF bf = sceneRect();
    if (1.0 * width() / height() > 1.0 * bf.width() / bf.height()) {
        return 1.0 * height() / bf.height();
    } else {
        return 1.0 * width() / bf.width();
    }
}

const QString ImageGraphicsView::path() const
{
    return m_path;
}

QPoint ImageGraphicsView::mapToImage(const QPoint &p) const
{
    return viewportTransform().inverted().map(p);
}

QRect ImageGraphicsView::mapToImage(const QRect &r) const
{
    return viewportTransform().inverted().mapRect(r);
}

QRect ImageGraphicsView::visibleImageRect() const
{
    return mapToImage(rect()) & QRect(0, 0, static_cast<int>(sceneRect().width()), static_cast<int>(sceneRect().height()));
}

bool ImageGraphicsView::isWholeImageVisible() const
{
    const QRect &r = visibleImageRect();
    const QRectF &sr = sceneRect();

    return r.width() >= sr.width() && r.height() >= sr.height();
}

bool ImageGraphicsView::isFitImage() const
{
    return m_isFitImage;
}

bool ImageGraphicsView::isFitWindow() const
{
    return m_isFitWindow;
}

void ImageGraphicsView::onImgFileChanged(const QString &ddfFile)
{
    Q_UNUSED(ddfFile)
    m_isChangedTimer->start(200);
}

void ImageGraphicsView::onLoadTimerTimeout()
{
    QFuture<QVariantList> f = QtConcurrent::run(m_pool, cachePixmap, m_loadPath);
    if (m_watcher.isRunning()) {
        m_watcher.cancel();
        m_watcher.waitForFinished();
    }
    m_watcher.setFuture(f);
    emit hideNavigation();
}

void ImageGraphicsView::onThemeTypeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::DarkType) {
        m_backgroundColor = utils::common::DARK_BACKGROUND_COLOR;
    } else {
        m_backgroundColor = utils::common::LIGHT_BACKGROUND_COLOR;
    }
    update();
}

void ImageGraphicsView::onIsChangedTimerTimeout()
{
    QFileInfo file(m_path);
    if (file.exists()) {
//        dApp->m_imageloader->updateImageLoader(QStringList(m_path));
        setImage(m_path);
        m_isChangedTimer->stop();
    } else {
        emit sigFIleDelete();
        m_isChangedTimer->stop();
    }
}

void ImageGraphicsView::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit doubleClicked();
    QGraphicsView::mouseDoubleClickEvent(e);
}

void ImageGraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);

    viewport()->setCursor(Qt::ArrowCursor);
    if (e->source() == Qt::MouseEventSynthesizedByQt && m_maxTouchPoints == 1) {
        const QRect &r = visibleImageRect();
        //double left=r.width()+r.x();
        const QRectF &sr = sceneRect();
        //fix 42660 2020/08/14 单指时间在QEvent处理，双指手势通过手势处理。为了解决图片放大后单指滑动手势冲突的问题
        if ((r.width() >= sr.width() && r.height() >= sr.height())) {
            int xpos = e->pos().x() - m_startpointx;
            if (abs(xpos) > 200 && m_startpointx != 0) {
                if (xpos > 0) {
                    emit previousRequested();
                } else {
                    emit nextRequested();
                }
            }
        }
    }
    m_startpointx = 0;
    m_maxTouchPoints = 0;
#ifdef tablet_PC
    if (m_press) {
        emit clicked();
    }
#endif
}

void ImageGraphicsView::mousePressEvent(QMouseEvent *e)
{
#ifdef tablet_PC
    m_press = true;
#endif
    QGraphicsView::mousePressEvent(e);

    viewport()->unsetCursor();
    viewport()->setCursor(Qt::ArrowCursor);

#ifndef tablet_PC
    emit clicked();
#endif
    m_startpointx = e->pos().x();
}

void ImageGraphicsView::mouseMoveEvent(QMouseEvent *e)
{
    m_press = false;
    if (!(e->buttons() | Qt::NoButton)) {
        viewport()->setCursor(Qt::ArrowCursor);

        emit mouseHoverMoved();
    } else {
        QGraphicsView::mouseMoveEvent(e);
        viewport()->setCursor(Qt::ClosedHandCursor);

        emit transformChanged();
    }
#ifndef tablet_PC
//    emit dApp->signalM->sigMouseMove(false);
#endif
}

void ImageGraphicsView::leaveEvent(QEvent *e)
{
//    dApp->getDAppNew()->restoreOverrideCursor();

    QGraphicsView::leaveEvent(e);
}

void ImageGraphicsView::resizeEvent(QResizeEvent *event)
{
    qDebug() << "---" << __FUNCTION__ << "---" << event->size();
    QGraphicsView::resizeEvent(event);
//    m_toast->move(width() / 2 - m_toast->width() / 2,
//                  height() - 80 - m_toast->height() / 2 - 11);
}

void ImageGraphicsView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

void ImageGraphicsView::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *mimeData = e->mimeData();
//    if (!utils::base::checkMimeData(mimeData)) {
//        return;
//    }
    e->accept();
}

void ImageGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
//    QPixmap pm(12, 12);
//    QPainter pmp(&pm);
//    //TODO: the transparent box
//    //should not be scaled with the image
//    pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
//    pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
//    pmp.end();

    painter->save();
    painter->fillRect(rect, m_backgroundColor);

//    QPixmap currentImage(m_path);
//    if (!currentImage.isNull())
//        painter->fillRect(currentImage.rect(), QBrush(pm));
    painter->restore();
}
int static count = 0;
bool ImageGraphicsView::event(QEvent *event)
{
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate ||
            evType == QEvent::TouchEnd) {
        if (evType == QEvent::TouchBegin) {
            count = 0;
            m_maxTouchPoints = 1;
        }
        if (evType == QEvent::TouchUpdate) {
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if (touchPoints.size() > count) {
                count = touchPoints.size();
            }
        }
        if (evType == QEvent::TouchEnd) {
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

            if (touchPoints.size() == 1 && count <= 1) {
                //QPointF centerPointOffset = gesture->centerPoint();
                qreal offset = touchPoints.at(0).lastPos().x() - touchPoints.at(0).startPos().x();
                if (qAbs(offset) > 200) {
                    if (offset > 0) {
                        emit previousRequested();
                        qDebug() << "zy------ImageView::event previousRequested";
                    } else {
                        emit nextRequested();
                        qDebug() << "zy------ImageView::event nextRequested";
                    }
                }
            }
        }
        /*lmh0804*/
//        const QRect &r = visibleImageRect();
//        double left = r.width() + r.x();
//        const QRectF &sr = sceneRect();
//        if (r.x() <= 1) {
//            return true;
//        }
//        if (left - sr.width() >= -1 && left - sr.width() <= 1) {
//            return true;
//        }
//        if (r.width() >= sr.width()) {
//            return true;
//        }
    } else if (evType == QEvent::Gesture)
        handleGestureEvent(static_cast<QGestureEvent *>(event));

    return QGraphicsView::event(event);
}

void ImageGraphicsView::onCacheFinish()
{
    QVariantList vl = m_watcher.result();
    if (vl.length() == 2) {
        const QString path = vl.first().toString();
        QPixmap pixmap = vl.last().value<QPixmap>();
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        if (path == m_path) {
            if (!m_pixmapItem)
                return;
            m_pixmapItem->setGraphicsEffect(nullptr);
            m_pixmapItem->setPixmap(pixmap);
            setSceneRect(m_pixmapItem->boundingRect());
            autoFit();
            emit imageChanged(path);
            this->update();
        }
    }
}

//void ImageView::onThemeChanged(ViewerThemeManager::AppTheme theme)
//{
//    if (theme == ViewerThemeManager::Dark) {
//        m_backgroundColor = utils::common::DARK_BACKGROUND_COLOR;
//        m_loadingIconPath = utils::view::DARK_LOADINGICON;
//    } else {
//        m_backgroundColor = utils::common::LIGHT_BACKGROUND_COLOR;
//        m_loadingIconPath = utils::view::LIGHT_LOADINGICON;
//    }
//    update();
//}

void ImageGraphicsView::scaleAtPoint(QPoint pos, qreal factor)
{
    // Remember zoom anchor point.
    const QPointF targetPos = pos;
    const QPointF targetScenePos = mapToScene(targetPos.toPoint());

    // Do the scaling.
    setScaleValue(factor);

    // Restore the zoom anchor point.
    //
    // The Basic idea here is we don't care how the scene is scaled or transformed,
    // we just want to restore the anchor point to the target position we've
    // remembered, in the coordinate of the view/viewport.
    const QPointF curPos = mapFromScene(targetScenePos);
    const QPointF centerPos = QPointF(width() / 2.0, height() / 2.0) + (curPos - targetPos);
    const QPointF centerScenePos = mapToScene(centerPos.toPoint());
    centerOn(centerScenePos.x(), centerScenePos.y());
}

void ImageGraphicsView::handleGestureEvent(QGestureEvent *gesture)
{
    if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void ImageGraphicsView::pinchTriggered(QPinchGesture *gesture)
{
    m_maxTouchPoints = 2;
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        QPoint pos = mapFromGlobal(gesture->centerPoint().toPoint());
        scaleAtPoint(pos, gesture->scaleFactor());
    }

    if (changeFlags & QPinchGesture::CenterPointChanged) {
        if (!m_isFirstPinch) {
            m_centerPoint = gesture->centerPoint();
            m_isFirstPinch = true;
        }
    }
    if (gesture->state() == Qt::GestureFinished) {
#ifndef tablet_PC
        QPointF centerPointOffset = gesture->centerPoint();
        qreal offset = centerPointOffset.x() - m_centerPoint.x();
        if (qAbs(offset) > 200) {
            if (offset > 0) {
                emit previousRequested();
                qDebug() << "zy------ImageView::pinchTriggered previousRequested";
            } else {
                emit nextRequested();
                qDebug() << "zy------ImageView::pinchTriggered nextRequested";
            }
        }
#endif
        m_isFirstPinch = false;
    }
}

void ImageGraphicsView::wheelEvent(QWheelEvent *event)
{
    QFileInfo file(m_path);
    if (!file.exists()) {
        event->accept();
    } else {
        qreal factor = qPow(1.2, event->delta() / 240.0);
        scaleAtPoint(event->pos(), factor);

        event->accept();
    }
    qDebug() << "---" << __FUNCTION__ << "---" << this->sceneRect();
}

//CFileWatcher::CFileWatcher(QObject *parent): QThread(parent)
//{
//    _handleId = inotify_init();
//}

//CFileWatcher::~CFileWatcher()
//{
//    clear();
//}

//bool CFileWatcher::isVaild()
//{
//    return (_handleId != -1);
//}

//void CFileWatcher::addWather(const QString &path)
//{
//    QMutexLocker loker(&_mutex);
//    if (!isVaild())
//        return;
//    QFileInfo info(path);
//    if (!info.exists() || !info.isFile()) {
//        return;
//    }
//    if (watchedFiles.find(path) != watchedFiles.end()) {
//        return;
//    }
//    std::string sfile = path.toStdString();
//    int fileId = inotify_add_watch(_handleId, sfile.c_str(), IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
//    watchedFiles.insert(path, fileId);
//    watchedFilesId.insert(fileId, path);
//    if (!_running) {
//        _running = true;
//        start();
//    }
//}

//void CFileWatcher::removePath(const QString &path)
//{
//    QMutexLocker loker(&_mutex);
//    if (!isVaild())
//        return;
//    auto itf = watchedFiles.find(path);
//    if (itf != watchedFiles.end()) {
//        inotify_rm_watch(_handleId, itf.value());
//        watchedFilesId.remove(itf.value());
//        watchedFiles.erase(itf);
//    }
//}

//void CFileWatcher::clear()
//{
//    QMutexLocker loker(&_mutex);
//    for (auto it : watchedFiles) {
//        inotify_rm_watch(_handleId, it);
//    }
//    watchedFilesId.clear();
//    watchedFiles.clear();
//}

//void CFileWatcher::run()
//{
//    doRun();
//}

//void CFileWatcher::doRun()
//{
//    if (!isVaild())
//        return;

//    char name[1024];
//    auto freadsome = [ = ](void *dest, size_t remain, FILE * file) {
//        char *offset = reinterpret_cast<char *>(dest);
//        while (remain) {
//            if (file == nullptr)
//                return -1;
//            size_t n = fread(offset, 1, remain, file);
//            if (n == 0) {
//                return -1;
//            }

//            remain -= n;
//            offset += n;
//        }
//        return 0;
//    };

//    FILE *watcher_file = fdopen(_handleId, "r");

//    while (true) {
//        inotify_event event;
//        if (-1 == freadsome(&event, sizeof(event), watcher_file)) {
//            qWarning() << "------------- freadsome error !!!!!---------- ";
//            break;
//        }
//        if (event.len) {
//            freadsome(name, event.len, watcher_file);
//        } else {
//            QMutexLocker loker(&_mutex);
//            auto itf = watchedFilesId.find(event.wd);
//            if (itf != watchedFilesId.end()) {
//                //qDebug() << "file = " << itf.value() << " event.wd = " << event.wd << "event.mask = " << event.mask;

//                if (event.mask & IN_MODIFY) {
//                    emit fileChanged(itf.value(), EFileModified);
//                } else if (event.mask & IN_MOVE_SELF) {
//                    emit fileChanged(itf.value(), EFileMoved);
//                } else if (event.mask & IN_DELETE_SELF) {
//                    emit fileChanged(itf.value(), EFileMoved);
//                }
//            }
//        }
//    }
//}
