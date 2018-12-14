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
#include "imageview.h"

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

#include "graphicsitem.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "utils/snifferimageformat.h"
#include "application.h"
#include "widgets/toast.h"
#include "widgets/dspinner.h"
#include <DSvgRenderer>

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

DWIDGET_USE_NAMESPACE

namespace
{

const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");

const qreal MAX_SCALE_FACTOR = 20.0;
const qreal MIN_SCALE_FACTOR = 0.02;
const QSize SPINNER_SIZE = QSize(32, 32);

QVariantList cachePixmap(const QString &path)
{
    QImage tImg;

    QString format = DetectImageFormat(path);
    if (format.isEmpty()) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            tImg = reader.read();
        }
    } else {
        QImageReader readerF(path, format.toLatin1());
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            tImg = readerF.read();
        } else {
            qWarning() << "can't read image:" << readerF.errorString()
                       << format;

            tImg = QImage(path);
        }
    }

    QPixmap p = QPixmap::fromImage(tImg);
    QVariantList vl;
    vl << QVariant(path) << QVariant(p);
    return vl;
}

}  // namespace

ImageView::ImageView(QWidget *parent)
    : QGraphicsView(parent)
    , m_renderer(Native)
    , m_pool(new QThreadPool())
    , m_svgItem(nullptr)
    , m_movieItem(nullptr)
    , m_pixmapItem(nullptr)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    setScene(new QGraphicsScene(this));
    setMouseTracking(true);
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);
    setAcceptDrops(false);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    grabGesture(Qt::PinchGesture);

    connect(&m_watcher, SIGNAL(finished()), this, SLOT(onCacheFinish()));
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ImageView::onThemeChanged);
    m_pool->setMaxThreadCount(1);

    m_toast = new Toast(this);
    m_toast->setIcon(":/resources/common/images/dialog_warning.svg");
    m_toast->setText(tr("This file contains multiple pages, please use Evince to view all pages."));
    m_toast->hide();
    // TODO
    //    QPixmap pm(12, 12);
    //    QPainter pmp(&pm);
    //    pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
    //    pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
    //    pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
    //    pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
    //    pmp.end();

    //    QPalette pal = palette();
    //    pal.setBrush(backgroundRole(), QBrush(pm));
    //    setAutoFillBackground(true);
    //    setPalette(pal);

    // Use openGL to render by default
    //    setRenderer(OpenGL);
}

void ImageView::clear()
{
    scene()->clear();
}

void ImageView::setImage(const QString &path)
{
    // Empty path will cause crash in release-build mode
    if (path.isEmpty()) {
        return;
    }
    m_path = path;
    QGraphicsScene *s = scene();

    QFileInfo fi(path);

    QString oldHintPath = m_toast->property("hint_path").toString();
    if (oldHintPath != fi.canonicalFilePath()) {
        m_toast->setProperty("hide_by_user", false);
    }
    m_toast->setProperty("hint_path", fi.canonicalFilePath());

    if (QFileInfo(path).suffix() == "tif" && !m_toast->property("hide_by_user").toBool()) {
        m_toast->show();
        m_toast->move(width() / 2 - m_toast->width() / 2,
                      height() - 80 - m_toast->height() / 2);
    } else {
        m_toast->hide();
    }

    // The suffix of svf file should be svg
    if (QFileInfo(path).suffix() == "svg" && DSvgRenderer().load(path)) {
        m_movieItem = nullptr;
        m_pixmapItem = nullptr;
        s->clear();
        resetTransform();
        m_svgItem = new QGraphicsSvgItem(path);
        m_svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
        m_svgItem->setCacheMode(QGraphicsItem::NoCache);
        m_svgItem->setZValue(0);
        // Make sure item show in center of view after reload
        setSceneRect(m_svgItem->boundingRect());
        s->addItem(m_svgItem);
        emit imageChanged(path);
    } else {
        m_svgItem = nullptr;
        // Support gif and mng
        if (QMovie(path).frameCount() > 1) {
            m_pixmapItem = nullptr;
            s->clear();
            resetTransform();
            m_movieItem = new GraphicsMovieItem(path);
            m_movieItem->start();
            // Make sure item show in center of view after reload
            setSceneRect(m_movieItem->boundingRect());
            s->addItem(m_movieItem);
            emit imageChanged(path);
        } else {
            m_movieItem = nullptr;
            QFuture<QVariantList> f = QtConcurrent::run(m_pool, cachePixmap, path);
            if (! m_watcher.isRunning()) {
                m_watcher.setFuture(f);

                //show loading gif.
                m_pixmapItem = nullptr;
                s->clear();
                resetTransform();
                GraphicsMovieItem *loadingItem = new GraphicsMovieItem(m_loadingIconPath);
                loadingItem->start();
                // Make sure item show in center of view after reload

                auto spinner = new DSpinner;
                spinner->setFixedSize(SPINNER_SIZE);
                spinner->setBackgroundColor(Qt::transparent);
                spinner->start();
                QWidget *w = new QWidget();
                w->setFixedSize(SPINNER_SIZE);
                QHBoxLayout *hLayout = new QHBoxLayout;
                hLayout->setMargin(0);
                hLayout->setSpacing(0);
                hLayout->addWidget(spinner, 0, Qt::AlignCenter);
                w->setLayout(hLayout);
                setSceneRect(w->rect());
                s->addWidget(w);

                emit hideNavigation();
            }
        }
    }
}

void ImageView::setRenderer(RendererType type)
{
    m_renderer = type;

    if (m_renderer == OpenGL) {
#ifndef QT_NO_OPENGL
        setViewport(new QOpenGLWidget());
#endif
    } else {
        setViewport(new QWidget);
    }
}

void ImageView::setScaleValue(qreal v)
{
    scale(v, v);
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

    emit scaled(imageRelativeScale() * 100);
    emit showScaleLabel();
    emit transformChanged();
}

const QImage ImageView::image()
{
    if (m_movieItem) {           // bit-map
        return m_movieItem->pixmap().toImage();
    } else if (m_pixmapItem) {
        //FIXME: access to m_pixmapItem will crash
        return m_pixmapItem->pixmap().toImage();
    } else if (m_svgItem) {    // svg
        QImage image(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
        QPainter imagePainter(&image);
        QGraphicsView::render(&imagePainter);
        imagePainter.end();
        return image;
    } else {
        return QImage();
    }
}

void ImageView::fitWindow()
{
    qreal wrs = windowRelativeScale();
    resetTransform();
    scale(wrs, wrs);
    m_isFitImage = false;
    m_isFitWindow = true;
    scaled(imageRelativeScale() * 100);
    emit transformChanged();
}

void ImageView::fitImage()
{
    resetTransform();
    scale(1, 1);
    m_isFitImage = true;
    m_isFitWindow = false;
    scaled(imageRelativeScale() * 100);
    emit transformChanged();
}

void ImageView::rotateClockWise()
{
    utils::image::rotate(m_path, 90);
    setImage(m_path);
}

void ImageView::rotateCounterclockwise()
{
    utils::image::rotate(m_path, - 90);
    setImage(m_path);
}

void ImageView::centerOn(int x, int y)
{
    QGraphicsView::centerOn(x, y);
    emit transformChanged();
}

qreal ImageView::imageRelativeScale() const
{
    // vertical scale factor are equal to the horizontal one
    return transform().m11();
}

qreal ImageView::windowRelativeScale() const
{
    QRectF bf = sceneRect();
    if (1.0 * width() / height() > 1.0 * bf.width() / bf.height()) {
        return 1.0 * height() / bf.height();
    } else {
        return 1.0 * width() / bf.width();
    }
}

const QRectF ImageView::imageRect() const
{
    QRectF br(mapFromScene(0, 0), sceneRect().size());
    QTransform tf = transform();
    br.translate(tf.dx(), tf.dy());
    br.setWidth(br.width() * tf.m11());
    br.setHeight(br.height() * tf.m22());

    return br;
}

const QString ImageView::path() const
{
    return m_path;
}

QPoint ImageView::mapToImage(const QPoint &p) const
{
    return viewportTransform().inverted().map(p);
}

QRect ImageView::mapToImage(const QRect &r) const
{
    return viewportTransform().inverted().mapRect(r);
}

QRect ImageView::visibleImageRect() const
{
    return mapToImage(rect()) & QRect(0, 0, sceneRect().width(), sceneRect().height());
}

bool ImageView::isWholeImageVisible() const
{
    return visibleImageRect().size() == sceneRect().size();
}

bool ImageView::isFitImage() const
{
    return m_isFitImage;
}

bool ImageView::isFitWindow() const
{
    return m_isFitWindow;
}

void ImageView::setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
    setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
    Q_UNUSED(highQualityAntialiasing);
#endif
}

void ImageView::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit doubleClicked();
    QGraphicsView::mouseDoubleClickEvent(e);
}

void ImageView::mouseReleaseEvent(QMouseEvent *e)
{
    if (!items().isEmpty()) {
        items().first()->setCursor(Qt::ArrowCursor);
        dApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
    }

    dApp->setOverrideCursor(QCursor(Qt::OpenHandCursor));
    QGraphicsView::mouseReleaseEvent(e);
}

void ImageView::mousePressEvent(QMouseEvent *e)
{
    emit clicked();

    if (!items().isEmpty()) {
        items().first()->setCursor(Qt::ArrowCursor);
    }

    QGraphicsView::mousePressEvent(e);
}

void ImageView::mouseMoveEvent(QMouseEvent *e)
{
    if (! (e->buttons() | Qt::NoButton)) {
        if (!items().isEmpty()) {
            items().first()->setCursor(Qt::ArrowCursor);
        }

        emit mouseHoverMoved();
    } else {
        if (! items().isEmpty()) {
            items().first()->setCursor(Qt::ClosedHandCursor);
        }

        emit transformChanged();
        QGraphicsView::mouseMoveEvent(e);
    }
}

void ImageView::leaveEvent(QEvent *e)
{
    dApp->restoreOverrideCursor();

    QGraphicsView::leaveEvent(e);
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    m_toast->move(width() / 2 - m_toast->width() / 2,
                  height() - 80 - m_toast->height() / 2);
}

void ImageView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

void ImageView::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
}

void ImageView::drawBackground(QPainter *painter, const QRectF &rect)
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

bool ImageView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        handleGestureEvent(static_cast<QGestureEvent*>(event));

    QGraphicsView::event(event);
}

void ImageView::onCacheFinish()
{
    QVariantList vl = m_watcher.result();
    if (vl.length() == 2) {
        const QString path = vl.first().toString();
        QPixmap pixmap = vl.last().value<QPixmap>();
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        if (path == m_path) {
            scene()->clear();
            resetTransform();
            m_pixmapItem = new GraphicsPixmapItem(pixmap);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
            // Make sure item show in center of view after reload
            setSceneRect(m_pixmapItem->boundingRect());
            scene()->addItem(m_pixmapItem);

            if ((width() < pixmap.width() || height() < pixmap.height())
                    && width() > 0 && height() > 0) {
                fitWindow();
            } else {
                fitImage();
            }

            emit imageChanged(path);
        }
    }
}

void ImageView::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    if (theme == ViewerThemeManager::Dark) {
        m_backgroundColor = utils::common::DARK_BACKGROUND_COLOR;
        m_loadingIconPath = utils::view::DARK_LOADINGICON;
    } else {
        m_backgroundColor = utils::common::LIGHT_BACKGROUND_COLOR;
        m_loadingIconPath = utils::view::LIGHT_LOADINGICON;
    }
    update();
}

void ImageView::scaleAtPoint(QPoint pos, qreal factor)
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

void ImageView::handleGestureEvent(QGestureEvent *gesture)
{
    if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void ImageView::pinchTriggered(QPinchGesture *gesture)
{
    QPoint pos = mapFromGlobal(gesture->centerPoint().toPoint());
    scaleAtPoint(pos, gesture->scaleFactor());
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    scaleAtPoint(event->pos(), factor);
}
