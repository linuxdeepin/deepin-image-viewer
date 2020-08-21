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

#include <qmath.h>
#include <QDebug>
#include <QFile>
#include <QGestureEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMovie>
#include <QOpenGLWidget>
#include <QPaintEvent>
#include <QScrollBar>
#include <QMetaType>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <QtConcurrent>
#include <QPainter>
#include <QTransform>
#include <QSvgGenerator>
#include <QScreen>
#include <QDesktopWidget>

#include <DGuiApplicationHelper>
#include <DSpinner>
#include <DSvgRenderer>

#include "application.h"
#include "controller/signalmanager.h"
#include "graphicsitem.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "utils/snifferimageformat.h"
#include "widgets/toast.h"
#include <malloc.h>

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

#ifdef USE_UNIONIMAGE
#include "unionimage.h"
#endif

DWIDGET_USE_NAMESPACE

namespace {

const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");

const QString ICON_PIXMAP_DARK = ":/assets/dark/images/picture damaged_dark.svg";
const QString ICON_PIXMAP_LIGHT = ":/assets/light/images/picture damaged_light.svg";

const qreal MAX_SCALE_FACTOR = 20.0;
const qreal MIN_SCALE_FACTOR = 0.029;
const QSize SPINNER_SIZE = QSize(40, 40);

//QVariantList cachePixmap(const QString path)
//{
//    QImage tImg;

//    QString format = DetectImageFormat(path);
//    if (format.isEmpty()) {
//        qDebug() << "开始读取缓存";
//        QImageReader reader(path);
//        qDebug() << "render结束";
//        reader.setAutoTransform(true);
//        if (reader.canRead()) {
//            qDebug() << "开始render读取到img";
//            tImg = reader.read();
//            qDebug() << "render读取到img完成";
//        }
//    } else {
//        qDebug() << "开始读取缓存";
//        QImageReader readerF(path, format.toLatin1());
//        qDebug() << "render结束";
//        readerF.setAutoTransform(true);
//        if (readerF.canRead()) {
//            qDebug() << "开始render读取到img";
//            tImg = readerF.read();
//            qDebug() << "render读取到img完成";
//        } else {
//            qWarning() << "can't read image:" << readerF.errorString() << format;

//            tImg = QImage(path);
//        }
//    }

//    QPixmap p = QPixmap::fromImage(tImg);
//    QVariantList vl;
//    vl << QVariant(path) << QVariant(p);
//    qDebug() << "render缓存结束";
//    return vl;
//}

}  // namespace

//开启线程加载原图
QVariantList cacheImage(const QString &path)
{
    QImage tImg;
    QString errMsg;
#if USE_UNIONIMAGE
    UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg);
#else
    QString format = DetectImageFormat(path);
            QImageReader readerF(path, format.toLatin1());
            readerF.setAutoTransform(true);
            if (readerF.canRead()) {
                tImg = readerF.read();
            } else {
                qWarning() << "can't read image:" << readerF.errorString() << format;

                tImg = QImage(path);
            }
#endif
      QPixmap p = QPixmap::fromImage(tImg);
    if (QFileInfo(path).exists() && p.isNull()) {
        //判定为损坏图片
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        QString picString;
        if (themeType == DGuiApplicationHelper::DarkType) {
            picString = ICON_PIXMAP_DARK;
        } else {
            picString = ICON_PIXMAP_LIGHT;
        }
#if USE_UNIONIMAGE
        UnionImage_NameSpace::loadStaticImageFromFile(picString, tImg, errMsg);
#else
        QImageReader readerF(picString, format.toLatin1());
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            tImg = readerF.read();
        } else {
            qWarning() << "can't read image:" << readerF.errorString() << format;

            tImg = QImage(picString);
        }
#endif
        qDebug() << errMsg;
        p = QPixmap::fromImage(tImg);
    }
    QVariantList vl;
    vl << QVariant(path) << QVariant(p);
    return vl;
}

QMimeType determineMimeType(const QString &filename)
{
    QMimeDatabase db;

    QFileInfo fileinfo(filename);
    QString inputFile = filename;

    // #328815: since detection-by-content does not work for compressed tar archives (see below why)
    // we cannot rely on it when the archive extension is wrong; we need to validate by hand.
    if (fileinfo.completeSuffix()
            .toLower()
            .remove(QRegularExpression(QStringLiteral("[^a-z\\.]")))
            .contains(QStringLiteral("tar."))) {
        inputFile.chop(fileinfo.completeSuffix().length());
        QString cleanExtension(fileinfo.completeSuffix().toLower());

        // tar.bz2 and tar.lz4 need special treatment since they contain numbers.
        bool isBZ2 = false;
        bool isLZ4 = false;
        if (fileinfo.completeSuffix().toLower().contains(QStringLiteral("bz2"))) {
            cleanExtension.remove(QStringLiteral("bz2"));
            isBZ2 = true;
        }
        if (fileinfo.completeSuffix().toLower().contains(QStringLiteral("lz4"))) {
            cleanExtension.remove(QStringLiteral("lz4"));
            isLZ4 = true;
        }

        // We remove non-alpha chars from the filename extension, but not periods.
        // If the filename is e.g. "foo.tar.gz.1", we get the "foo.tar.gz." string,
        // so we need to manually drop the last period character from it.
        cleanExtension.remove(QRegularExpression(QStringLiteral("[^a-z\\.]")));
        if (cleanExtension.endsWith(QLatin1Char('.'))) {
            cleanExtension.chop(1);
        }

        // Re-add extension for tar.bz2 and tar.lz4.
        if (isBZ2) {
            cleanExtension.append(QStringLiteral(".bz2"));
        }
        if (isLZ4) {
            cleanExtension.append(QStringLiteral(".lz4"));
        }

        inputFile += cleanExtension;
    }

    QMimeType mimeFromExtension = db.mimeTypeForFile(inputFile, QMimeDatabase::MatchExtension);
    QMimeType mimeFromContent = db.mimeTypeForFile(filename, QMimeDatabase::MatchContent);

    qDebug() << mimeFromExtension.name() << "FE xxxxx FC" << mimeFromContent.name();
    // mimeFromContent will be "application/octet-stream" when file is
    // unreadable, so use extension.
    if (!fileinfo.isReadable()) {
        return mimeFromExtension;
    }

    // Compressed tar-archives are detected as single compressed files when
    // detecting by content. The following code fixes detection of tar.gz, tar.bz2, tar.xz,
    // tar.lzo, tar.lz, tar.lrz and tar.zst.
    if ((mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-compressed-tar")) &&
            mimeFromContent == db.mimeTypeForName(QStringLiteral("application/gzip"))) ||
            (mimeFromExtension ==
             db.mimeTypeForName(QStringLiteral("application/x-bzip-compressed-tar")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-bzip"))) ||
            (mimeFromExtension ==
             db.mimeTypeForName(QStringLiteral("application/x-xz-compressed-tar")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-xz"))) ||
            (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-tarz")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-compress"))) ||
            (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-tzo")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lzop"))) ||
            (mimeFromExtension ==
             db.mimeTypeForName(QStringLiteral("application/x-lzip-compressed-tar")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lzip"))) ||
            (mimeFromExtension ==
             db.mimeTypeForName(QStringLiteral("application/x-lrzip-compressed-tar")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lrzip"))) ||
            (mimeFromExtension ==
             db.mimeTypeForName(QStringLiteral("application/x-lz4-compressed-tar")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lz4"))) ||
            (mimeFromExtension ==
             db.mimeTypeForName(QStringLiteral("application/x-zstd-compressed-tar")) &&
             mimeFromContent == db.mimeTypeForName(QStringLiteral("application/zstd")))) {
        return mimeFromExtension;
    }

    if (mimeFromExtension != mimeFromContent) {
        if (mimeFromContent.isDefault()) {
            return mimeFromExtension;
        }

        // #354344: ISO files are currently wrongly detected-by-content.
        if (mimeFromExtension.inherits(QStringLiteral("application/x-cd-image"))) {
            return mimeFromExtension;
        }
    }

    return mimeFromContent;
}

QVariantList ImageView::cachePixmap(const QString path)
{
    using namespace utils;
    using namespace image;
#ifdef USE_UNIONIMAGE

    QImage tImg;
    QString errMsg;
    if (!imageSupportRead(path)){
        tImg= QImage();
    }
    else if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
        qDebug() << errMsg;
    }
    QPixmap p = QPixmap::fromImage(tImg);
    if (QFileInfo(path).exists() && p.isNull()) {
        //判定为损坏图片
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        QString picString;
        if (themeType == DGuiApplicationHelper::DarkType) {
            picString = ICON_PIXMAP_DARK;
        } else {
            picString = ICON_PIXMAP_LIGHT;
        }
        UnionImage_NameSpace::loadStaticImageFromFile(picString, tImg, errMsg);
        p = QPixmap::fromImage(tImg);
    }
#else
    QImage tImg;
    QString format = DetectImageFormat(path);
    if (format.isEmpty()) {
        qDebug() << "render开始";
        QImageReader reader(path);
        qDebug() << "render结束";
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            qDebug() << "开始render读取到img";
            tImg = reader.read();
            qDebug() << "render读取到img完成";
        }
    } else {
        qDebug() << "开始读取缓存";
        QImageReader readerF(path, format.toLatin1());
        qDebug() << "render结束";
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            qDebug() << "开始render读取到img";
            tImg = readerF.read();
            qDebug() << "render读取到img完成";
        } else {
            qWarning() << "can't read image:" << readerF.errorString() << format;

            tImg = QImage(path);
        }
    }

    QPixmap p = QPixmap::fromImage(tImg);
#endif
    QVariantList vl;
    vl << QVariant(path) << QVariant(p);
    qDebug() << "render缓存结束";
    emit cacheThreadEndSig(vl);
    return vl;
}

ImageView::ImageView(QWidget *parent)
    : QGraphicsView(parent)
    , m_renderer(Native)
    , m_pool(new QThreadPool())
    , m_imgSvgItem(nullptr)
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
    setFrameShape(QFrame::Shape::NoFrame);

    viewport()->setCursor(Qt::ArrowCursor);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(showFileImage()));
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ImageView::onThemeChanged);
    m_pool->setMaxThreadCount(1);
    connect(this, &ImageView::cacheThreadEndSig, this, &ImageView::onCacheFinish);

    m_toast = new Toast(this);
    m_toast->setIcon(":/assets/common/images/dialog_warning.svg");
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
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ]() {
        DGuiApplicationHelper::ColorType themeType =
            DGuiApplicationHelper::instance()->themeType();
        if (themeType == DGuiApplicationHelper::DarkType) {
            m_backgroundColor = utils::common::DARK_BACKGROUND_COLOR;
        } else {
            m_backgroundColor = utils::common::LIGHT_BACKGROUND_COLOR;
        }
        update();
    });
    connect(dApp->signalM, &SignalManager::loadingDisplay, this, [ = ](bool immediately) {
        if (immediately) {
            qDebug() << "loading display...";
            m_loadingDisplay = true;
        }
    });
    connect(&m_timerLoadPixmap,SIGNAL(timeout()),this,SLOT(startLoadPixmap()));
    m_timerLoadPixmap.start(300);
}

void ImageView::clear()
{
    scene()->clear();
}

void ImageView::setImage(const QString path)
{
    //LMH0610清理堆空间
     malloc_trim(0);
    // Empty path will cause crash in release-build mode
    if (path.isEmpty()) {
        return;
    }
    QFileInfo fi(path);

    emit dApp->signalM->enterView(true);
    qDebug() << "emit dApp->signalM->enterView(true)..................ImageView";
    qDebug() << "Path = " << path;
    qDebug() << "FileType = " << determineMimeType(path);
    //    if(path == m_path)return;//Add for no repeat refresh, delete for rotation no
    //    refresh(bugID3926)

    //heyi test  识别是否切换了图片，并判定上一张图片旋转状态是否发生了改变
    rotatePixCurrent();

    m_path = path;

    QString oldHintPath = m_toast->property("hint_path").toString();
    if (oldHintPath != fi.canonicalFilePath()) {
        m_toast->setProperty("hide_by_user", false);
    }
    m_toast->setProperty("hint_path", fi.canonicalFilePath());

    if (QFileInfo(path).suffix() == "tif" && !m_toast->property("hide_by_user").toBool()) {
        //        m_toast->show();
        m_toast->move(width() / 2 - m_toast->width() / 2,
                      height() - 80 - m_toast->height() / 2 - 11);
    } else {
        m_toast->hide();
    }

    PICTURE_TYPE type = judgePictureType(path);
    loadPictureByType(type, path);
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
    //由于矩阵被旋转，通过矩阵获取缩放因子，计算缩放比例错误，因此记录过程中的缩放因子来判断缩放比例
    m_scal *=v;
    qDebug() << m_scal;
    scale(v, v);
    //const qreal irs = imageRelativeScale() * devicePixelRatioF();
    // Rollback
    if (v < 1 && /*irs <= MIN_SCALE_FACTOR)*/m_scal<0.03) {
        const qreal minv = MIN_SCALE_FACTOR / m_scal;
        // if (minv < 1.09) return;
        scale(minv, minv);
        m_scal *=minv;
    } else if (v > 1 && /*irs >= MAX_SCALE_FACTOR*/m_scal>20) {
        const qreal maxv = MAX_SCALE_FACTOR / m_scal;
        scale(maxv, maxv);
        m_scal *=maxv;
    } else {
        m_isFitImage = false;
        m_isFitWindow = false;
    }

    qreal rescale = imageRelativeScale() * devicePixelRatioF();
    if (rescale - 1 > -0.01 && rescale - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }

    emit scaled(/*imageRelativeScale() * devicePixelRatioF() * 100*/m_scal*100);
    emit showScaleLabel();
    emit transformChanged();

    titleBarControl();
}

void ImageView::autoFit()
{
    if (image().isNull())
        return;

    QSize image_size = image().size();

    // change some code in graphicsitem.cpp line100.

    if ((image_size.width() >= width() || image_size.height() >= height() - 150) && width() > 0 &&
            height() > 0) {
        fitWindow();
    } else {
        fitImage();
    }

    titleBarControl();
}

void ImageView::titleBarControl()
{
    qDebug() << "imageHeight:"
             << image().size().height() * imageRelativeScale() * devicePixelRatioF()
             << image().size().height() * imageRelativeScale() << "&&&&&&"
             << "currentHeight" << height() << "currentRatio" << devicePixelRatioF();

    qreal realHeight = 0.0;

    if (m_movieItem || m_imgSvgItem) {
        realHeight = image().size().height() * imageRelativeScale() * devicePixelRatioF();

    } else {
        realHeight = image().size().height() * imageRelativeScale();
    }

    if (realHeight > height() - 100) {
        dApp->signalM->sigImageOutTitleBar(true);
    } else {
        dApp->signalM->sigImageOutTitleBar(false);
    }
}

const QImage ImageView::image(bool brefresh)
{
       /*lmh0807,解决崩溃的问题*/
    if (m_pixmapItem) {
        // FIXME: access to m_pixmapItem will crash
        return m_pixmapItem->pixmap().toImage();
        //    } else if (m_svgItem) {    // svg
    }
    else if (m_movieItem) {  // bit-map
        return m_movieItem->pixmap().toImage();
        //        return m_movieItem->getMovie()->currentImage();
    }


    else  if (m_imgSvgItem) {  // svg
        if(brefresh)
        {
            QImage image(m_imgSvgItem->renderer()->defaultSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(QColor(0, 0, 0, 0));
            QPainter imagePainter(&image);
            m_imgSvgItem->renderer()->render(&imagePainter);
            imagePainter.end();
            m_svgimg = image;
        }
        return m_svgimg;
    } else {
        return QImage();
    }
}

void ImageView::fitWindow()
{
    qreal wrs = windowRelativeScale();
    m_scal = wrs;
    resetTransform();
    scale(wrs, wrs);

    if (wrs - 1 > -0.01 && wrs - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }
    m_isFitImage = false;
    m_isFitWindow = true;
    scaled(imageRelativeScale() * devicePixelRatioF() * 100);
    emit transformChanged();
}

void ImageView::fitWindow_btnclicked()
{
    qreal wrs = windowRelativeScale_origin();
    resetTransform();
    m_scal = wrs;
    scale(wrs, wrs);
    if (wrs - 1 > -0.01 && wrs - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }
    m_isFitImage = false;
    m_isFitWindow = true;
    scaled(imageRelativeScale() * devicePixelRatioF() * 100);
    emit transformChanged();
}

void ImageView::fitImage()
{
    resetTransform();
    /** change ratio from 1 to 0.9, because one of test pictures is so special that cannot use
    * fitwindow() when use fitImage() picture bottom is over toolbar. so use 0.9 instead. 12-19,bug
    * 9839, change 0.9->1
    */
    m_scal = 1.0;
    scale(1, 1);
    emit checkAdaptImageBtn();
    m_isFitImage = true;
    m_isFitWindow = false;
    scaled(imageRelativeScale() * devicePixelRatioF() * 100);
    emit transformChanged();
}

void ImageView::rotateClockWise()
{
    if (QFileInfo(m_path).suffix() == "svg") {
        reloadSvgPix(m_path, 90);
    } else {
        rotatePixmap(90);
    }
}

void ImageView::rotateCounterclockwise()
{
    if (QFileInfo(m_path).suffix() == "svg") {
        reloadSvgPix(m_path, -90);
    } else {
        rotatePixmap(-90);
    }
}

void ImageView::centerOn(int x, int y)
{
    QGraphicsView::centerOn(x, y);
    emit transformChanged();
}

qreal ImageView::imageRelativeScale() const
{
    // vertical scale factor are equal to the horizontal one
    return transform().m11() / devicePixelRatioF();
}

qreal ImageView::windowRelativeScale() const
{
    QRectF bf = sceneRect();
    if (this->window()->isFullScreen()) {
        if (1.0 * (width()) / (height() + 15) > 1.0 * bf.width() / bf.height()) {
            return 1.0 * (height() + 15) / bf.height();
        } else {
            return 1.0 * (width()) / bf.width();
        }
    } else {
        if (1.0 * (width() - 20) / (height() - 180) > 1.0 * bf.width() / bf.height()) {
            return 1.0 * (height() - 180) / bf.height();
        } else {
            return 1.0 * (width() - 20) / bf.width();
        }
    }
}

qreal ImageView::windowRelativeScale_origin() const
{
    QRectF bf = sceneRect();
    if (this->window()->isFullScreen()) {
        if (1.0 * (width()) / (height()) > 1.0 * bf.width() / bf.height()) {
            return 1.0 * (height()) / bf.height();
        } else {
            return 1.0 * (width()) / bf.width();
        }
    } else {
        if (1.0 * (width()) / (height()) > 1.0 * bf.width() / bf.height()) {
            return 1.0 * (height()) / bf.height();
        } else {
            return 1.0 * (width()) / bf.width();
        }
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

void ImageView::setPath(const QString path)
{
    m_path = path;
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
    const QRect &r = visibleImageRect();
    const QRectF &sr = sceneRect();

    return r.width() >= sr.width() && r.height() >= sr.height();
}

bool ImageView::isFitImage() const
{
    return m_isFitImage;
}

bool ImageView::isFitWindow() const
{
    return m_isFitWindow;
}

void ImageView::rotatePixCurrent()
{
    if (0 != m_rotateAngel) {
        m_rotateAngel =  m_rotateAngel % 360;
        if (0 != m_rotateAngel) {
            utils::image::rotate(m_path, m_rotateAngel);
            m_rotateAngel = 0;
        }
    }
}

void ImageView::cacheThread(const QString strPath)
{
    QFileInfo fi(strPath);
    if (fi.suffix().toLower() == "svg") {
        QSvgRenderer svgRenderer;
        svgRenderer.load(strPath);
        m_rwCacheLock.lockForWrite();
        //m_hsSvg.insert(strPath, svgRenderer);
        m_rwCacheLock.unlock();
    } else if (fi.suffix().toLower() == "mng" || fi.suffix().toLower() == "gif"
               || fi.suffix().toLower() == "webp") {
        GraphicsMovieItem movieItem(strPath, fi.suffix());
        m_rwCacheLock.lockForWrite();
        //m_hsMovie.insert(strPath, movieItem);
        m_rwCacheLock.unlock();

    } else {
        QImage tImg;

        QString format = DetectImageFormat(strPath);
        if (format.isEmpty()) {
            QImageReader reader(strPath);
            reader.setAutoTransform(true);
            if (reader.canRead()) {
                tImg = reader.read();
            }
        } else {
            QImageReader readerF(strPath, format.toLatin1());
            readerF.setAutoTransform(true);
            if (readerF.canRead()) {
                tImg = readerF.read();
            } else {
                qWarning() << "can't read image:" << readerF.errorString() << format;

                tImg = QImage(strPath);
            }
        }

        QPixmap p = QPixmap::fromImage(tImg);
        m_rwCacheLock.lockForWrite();
        m_hsPixap.insert(strPath, p);
        m_rwCacheLock.unlock();
    }
}

//void ImageView::showPixmap(QString path)
//{
//    m_rwCacheLock.lockForRead();
//    QPixmap pixmap = m_hsPixap.value(path);
//    m_rwCacheLock.unlock();
//    pixmap.setDevicePixelRatio(devicePixelRatioF());
//    if (path == m_path) {
//        scene()->clear();
//        resetTransform();
//        m_pixmapItem = new GraphicsPixmapItem(pixmap);
//        m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
//        connect(dApp->signalM, &SignalManager::enterScaledMode, this, [ = ](bool scaledmode) {
//            if (!m_pixmapItem) {
//                qDebug() << "onCacheFinish.............m_pixmapItem=" << m_pixmapItem;
//                update();
//                return;
//            }
//            if (scaledmode) {
//                m_pixmapItem->setTransformationMode(Qt::FastTransformation);
//            } else {
//                m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
//                //m_pixmapItem->setTransformationMode(Qt::FastTransformation);
//            }
//        });
//        // Make sure item show in center of view after reload
//        QRectF rect = m_pixmapItem->boundingRect();
//        //            rect.setHeight(rect.height() + 50);
//        setSceneRect(rect);
//        //            setSceneRect(m_pixmapItem->boundingRect());
//        scene()->addItem(m_pixmapItem);

//        autoFit();

//        emit imageChanged(path);
//    }
//}

ImageView::PICTURE_TYPE ImageView::judgePictureType(const QString strPath)
{
    PICTURE_TYPE pixType = NORMAL;
    if (strPath.isEmpty()) {
        return pixType;
    }

    QFileInfo fi(strPath);
    QString strType = fi.suffix().toLower();
    if (strType == "svg" && DSvgRenderer().load(strPath)) {
        pixType = PICTURE_TYPE::SVG;
    } else if (strType == "mng" || strType == "gif"
               || strType == "webp") {
        pixType = PICTURE_TYPE::KINETOGRAM;
    } else {
        pixType = PICTURE_TYPE::NORMAL;
    }

    return pixType;
}

bool ImageView::loadPictureByType(ImageView::PICTURE_TYPE type, const QString strPath)
{
    bool bRet = true;
    QGraphicsScene *s = scene();
    m_bRoate = UnionImage_NameSpace::isImageSupportRotate(strPath) && QFileInfo(strPath).isWritable();
    switch (type) {
    case PICTURE_TYPE::SVG:
    case PICTURE_TYPE::NORMAL: {
        m_movieItem = nullptr;
        qDebug() << "cache start!";
        if (m_hsPixap.contains(strPath)) {
            //showPixmap(strPath);
            //fix 26153
            emit dApp->signalM->hideNavigation();
        } else {
            //QFuture<QVariantList> f = QtConcurrent::run(m_pool, cachePixmap, strPath);
            QThread *th = QThread::create([ = ]() {
                cachePixmap(m_path);
            });

            connect(th, &QThread::finished, th, &QObject::deleteLater);
            th->start();

//            static bool haha = false;
//            if (!haha) {
//                th->start();
//                haha = true;
//            }

            if (!m_watcher.isRunning()) {
                //                m_watcher.setFuture(f);

                if (m_loadingDisplay) {
                    m_loadingDisplay = false;

                    // show loading gif.
                    m_pixmapItem = nullptr;
                    s->clear();
                    resetTransform();

                    auto spinner = new DSpinner;
                    spinner->setFixedSize(SPINNER_SIZE);
                    spinner->start();
                    QWidget *w = new QWidget();
                    w->setFixedSize(SPINNER_SIZE);
                    QHBoxLayout *hLayout = new QHBoxLayout;
                    hLayout->setMargin(0);
                    hLayout->setSpacing(0);
                    hLayout->addWidget(spinner, 0, Qt::AlignCenter);
                    w->setLayout(hLayout);

                    // Make sure item show in center of view after reload
                    setSceneRect(w->rect());
                    s->addWidget(w);
                }

//                f.waitForFinished();
//                m_watcher.setFuture(f);
            }

            emit dApp->signalM->hideNavigation();
        }
        break;
    }

    case PICTURE_TYPE::KINETOGRAM: {
        m_pixmapItem = nullptr;
        s->clear();
        resetTransform();
        m_movieItem = new GraphicsMovieItem(strPath, QFileInfo(strPath).suffix());
        m_movieItem->start();
        // Make sure item show in center of view after reload
        setSceneRect(m_movieItem->boundingRect());
        s->addItem(m_movieItem);
        //LMH0603解决svg和gif和mng缩略图不显示问题
        if(dApp->m_firstLoad)
        {
            QThread *th = QThread::create([ = ]() {
                emit imageChanged(strPath);
              //  bool firstLoad = false;
              //  if (!firstLoad) {
                qDebug() << "load cache";
                    emit cacheEnd();
                   // firstLoad = true;
             //   }
            });

            connect(th, &QThread::finished, th, &QObject::deleteLater);
            th->start();
            dApp->m_firstLoad =false;

        }else {
            emit imageChanged(strPath);
}
        break;
    }

    }
    return bRet;
}

void ImageView::setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
    setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
    Q_UNUSED(highQualityAntialiasing);
#endif
}

void ImageView::endApp()
{
    if (!m_path.isEmpty()) {
        if (0 != m_rotateAngel) {
            m_rotateAngel =  m_rotateAngel % 360;
            if (0 != m_rotateAngel) {
                utils::image::rotate(m_path, m_rotateAngel);
                m_rotateAngel = 0;
            }
        }
    }
}

bool ImageView::reloadSvgPix(QString strPath, int nAngel,bool fitauto)
{
    bool bRet = true;
    QMatrix leftmatrix;
    QImage img(strPath);
    leftmatrix.rotate(nAngel);
    img=img.transformed(leftmatrix,Qt::SmoothTransformation);
    QSvgGenerator generator;
    generator.setFileName(strPath);
    generator.setViewBox(img.rect());
    QPainter painter;
    painter.begin(&generator);
    painter.drawImage(img.rect(),img);
    painter.end();
    if(fitauto)
        setImage(strPath);
    return  bRet;
}

void ImageView::rotatePixmap(int nAngel)
{
    if(!m_pixmapItem) return;
    QPixmap pixmap = m_pixmapItem->pixmap();
    QMatrix rotate;
    rotate.rotate(nAngel);

    pixmap = pixmap.transformed(rotate, Qt::FastTransformation);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    scene()->clear();
    resetTransform();
    m_pixmapItem = new GraphicsPixmapItem(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    // Make sure item show in center of view after reload
    QRectF rect = m_pixmapItem->boundingRect();
    //            rect.setHeight(rect.height() + 50);
    setSceneRect(rect);
    //            setSceneRect(m_pixmapItem->boundingRect());
    scene()->addItem(m_pixmapItem);

    autoFit();
    m_rotateAngel += nAngel;
}

void ImageView::recvPathsToCache(const QStringList pathsList)
{
    m_rwCacheLock.lockForWrite();
    m_pathsList = removeDiff(pathsList);
    m_rwCacheLock.unlock();

    QThread *th = QThread::create([ = ]() {
        QStringList list;
        m_rwCacheLock.lockForRead();
        list = m_pathsList;
        m_rwCacheLock.unlock();

        foreach (QString strPath, list) {
            if (strPath.isEmpty()) {
                continue;
            }

            cacheThread(strPath);
        }

        QThread::currentThread()->quit();
        qDebug() << "缓存结束时间：";

    });

    connect(th, &QThread::finished, th, &QObject::deleteLater);
    th->start();
}

void ImageView::delCacheFromPath(const QString strPath)
{
    if (strPath.isEmpty()) {
        return;
    }

    m_hsPixap.remove(strPath);
}

void ImageView::delAllCache()
{
    m_hsPixap.clear();
}

QStringList ImageView::removeDiff(QStringList pathsList)
{
    QHash<QString, QPixmap> hsPixap;
    QStringList loadPaths;
    foreach (QString strPath, pathsList) {
        if (m_hsPixap.contains(strPath)) {
            hsPixap.insert(strPath, m_hsPixap.value(strPath));
        } else {
            loadPaths.append(strPath);
        }
    }

    m_hsPixap.clear();
    m_hsPixap = hsPixap;

    return loadPaths;
}

void ImageView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        emit doubleClicked();
    QGraphicsView::mouseDoubleClickEvent(e);
}

void ImageView::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);

    viewport()->setCursor(Qt::ArrowCursor);
    if(e->source() == Qt::MouseEventSynthesizedByQt && m_maxTouchPoints==1)
    {
        const QRect &r = visibleImageRect();
        //double left=r.width()+r.x();
        const QRectF &sr = sceneRect();
        //fix 42660 2020/08/14 单指时间在QEvent处理，双指手势通过手势处理。为了解决图片放大后单指滑动手势冲突的问题
        if((r.width()>=sr.width() &&r.height()>=sr.height())){
            int xpos = e->pos().x()-m_startpointx;
            if(abs(xpos)> 200 && m_startpointx!= 0)
            {
                if(xpos>0)
                {
                    emit previousRequested();
                }else {
                    emit nextRequested();
                }
            }
        }
    }
    m_startpointx=0;
}

void ImageView::mousePressEvent(QMouseEvent *e)
{
    QGraphicsView::mousePressEvent(e);
    viewport()->unsetCursor();
    viewport()->setCursor(Qt::ArrowCursor);

    emit clicked();
    m_startpointx = e->pos().x();
}

void ImageView::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() | Qt::NoButton)) {
        viewport()->setCursor(Qt::ArrowCursor);

        emit mouseHoverMoved();
    } else {
        QGraphicsView::mouseMoveEvent(e);
        viewport()->setCursor(Qt::ClosedHandCursor);

        emit transformChanged();
    }
    emit dApp->signalM->sigMouseMove();
}

void ImageView::leaveEvent(QEvent *e)
{
    dApp->restoreOverrideCursor();

    QGraphicsView::leaveEvent(e);
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    qDebug() << "ImageView::resizeEvent";
    m_toast->move(width() / 2 - m_toast->width() / 2, height() - 80 - m_toast->height() / 2 - 11);

    // when resize window, make titlebar changed.
    if (!image().isNull()) {

        titleBarControl();
    }

    QGraphicsView::resizeEvent(event);
}

void ImageView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

void ImageView::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
    e->acceptProposedAction();
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
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate ||
            evType == QEvent::TouchEnd) {
        if(evType == QEvent::TouchBegin){
            m_maxTouchPoints=1;
        }
        else if(evType == QEvent::TouchUpdate){
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if(touchPoints.size()>m_maxTouchPoints){
                m_maxTouchPoints=touchPoints.size();
            }
        }else if (evType == QEvent::TouchEnd) {
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

            if (m_maxTouchPoints <= 1 /*||m_maxTouchPoints > 2*/) {
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
        const QRect &r = visibleImageRect();
        //double left=r.width()+r.x();
        const QRectF &sr = sceneRect();
        //fix 42660 2020/08/14 单指时间在QEvent处理，双指手势通过手势处理。为了解决图片放大后单指滑动手势冲突的问题
        if((r.width()>=sr.width() &&r.height()>=sr.height())){
            //return true;
        }
//        if((left-sr.width()>=-1 &&left-sr.width()<=1) ||(r.x()<=1) ||(r.width() >= sr.width())){
//            return true;
//        }
    } else if (event->type() == QEvent::Gesture)
        handleGestureEvent(static_cast<QGestureEvent *>(event));

    return QGraphicsView::event(event);
}

void ImageView::onCacheFinish(QVariantList vl)
{
    qDebug() << "cache end!";
    QScreen *screen = QGuiApplication::primaryScreen ();
    QRect mm = screen->availableGeometry() ;
    int screen_width = mm.width();
    int screen_height = mm.height();
    qDebug() << screen_width << screen_height;

    //QVariantList vl = m_watcher.result();
    if (vl.length() == 2) {
        const QString path = vl.first().toString();
        QPixmap pixmap = vl.last().value<QPixmap>();
        vl.clear();
       // pixmap = pixmap.scaled(screen_width, screen_height, Qt::KeepAspectRatio);
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        if (path == m_path) {
            scene()->clear();
            resetTransform();
            m_pixmapItem = new GraphicsPixmapItem(pixmap);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
            connect(dApp->signalM, &SignalManager::enterScaledMode, this, [ = ](bool scaledmode) {
                if (!m_pixmapItem) {
                    qDebug() << "onCacheFinish.............m_pixmapItem=" << m_pixmapItem;
                    update();
                    return;
                }
                if (scaledmode) {
                    m_pixmapItem->setTransformationMode(Qt::FastTransformation);
                } else {
                    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
                    //m_pixmapItem->setTransformationMode(Qt::FastTransformation);
                }
            });
            // Make sure item show in center of view after reload
            QRectF rect = m_pixmapItem->boundingRect();
            //            rect.setHeight(rect.height() + 50);
            setSceneRect(rect);
            //            setSceneRect(m_pixmapItem->boundingRect());
            scene()->addItem(m_pixmapItem);

            autoFit();

            emit imageChanged(path);
            //static bool firstLoad = false;
            if (dApp->m_firstLoad) {
                qDebug() << "load cache";
                emit cacheEnd();
                dApp->m_firstLoad =false;
               // firstLoad = true;
            }

            //将缓存的图片加入hash
//            if (!m_hsPixap.contains(path)) {
//                m_hsPixap.insert(path, pixmap);
//            }
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
    centerOn(static_cast<int>(centerScenePos.x()), static_cast<int>(centerScenePos.y()));
}

void ImageView::handleGestureEvent(QGestureEvent *gesture)
{
/*    if (QGesture *swipe = gesture->gesture(Qt::SwipeGesture))
        swipeTriggered(static_cast<QSwipeGesture *>(swipe));
    else */if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void ImageView::pinchTriggered(QPinchGesture *gesture)
{
    //    QPoint pos = mapFromGlobal(gesture->centerPoint().toPoint());
    //    scaleAtPoint(pos, gesture->scaleFactor());
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    m_maxTouchPoints = 2;
    //缩放手势
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        QPoint pos = mapFromGlobal(gesture->centerPoint().toPoint());
        if(abs(gesture->scaleFactor()-1)>0.006)
        {
            qDebug() << "scaleFactor" << gesture->scaleFactor();
            scaleAtPoint(pos, gesture->scaleFactor());
        }
    }
    //旋转手势
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        if(!m_bRoate || m_maxTouchPoints>2) return;
        //释放手指后旋转的位置未结束不能进行下次旋转
        if(!m_rotateflag)
        {
            qDebug() << "ratateflag" << gesture->lastRotationAngle();
            gesture->setRotationAngle(gesture->lastRotationAngle());
            return;
        }
        qreal rotationDelta = gesture->rotationAngle() - gesture->lastRotationAngle();
        //防止在旋转过程中触发切换到下一张
        if(abs(gesture->rotationAngle())>20) m_bnextflag = false;
        if(abs(rotationDelta)>0.2)
        {
            m_rotateAngelTouch = gesture->rotationAngle();
            rotate(rotationDelta);
        }
    }

    if (changeFlags & QPinchGesture::CenterPointChanged) {
        if (!isFirstPinch) {
            centerPoint = gesture->centerPoint();
            isFirstPinch = true;
        }
    }
    if (gesture->state() == Qt::GestureFinished) {
        isFirstPinch = false;
        gesture->setCenterPoint(centerPoint);
      //旋转松开手势操作
       // m_rotateAngelTouch = m_rotateAngelTouch % 360;
        //int abs(m_rotateAngelTouch);
        if(!m_bRoate) return;
        m_rotateflag = false;
        QPropertyAnimation *animation = new QPropertyAnimation(this, "rotation");
         animation->setDuration(200);
         if(m_rotateAngelTouch<0) m_rotateAngelTouch += 360;
         qreal endvalue;
         if(abs(0-abs(m_rotateAngelTouch))<=10)
         {
            endvalue = 0;
         }else if(abs(360-abs(m_rotateAngelTouch))<=10)
         {
             endvalue = 0;
         }else if(abs(90-abs(m_rotateAngelTouch))<=10)
         {
             endvalue = 90;
         }else if(abs(180-abs(m_rotateAngelTouch))<=10)
         {
             endvalue = 180;
         }else if(abs(270-abs(m_rotateAngelTouch))<=10)
         {
             endvalue = 270;
         }else {
            endvalue = 0;
         }
         if(!m_bRoate) endvalue = 0;
         m_endvalue=endvalue;
         qreal startvalue;
         if(abs(m_rotateAngelTouch-endvalue)>180)
         {
             startvalue = m_rotateAngelTouch-360;
         }else {
             startvalue =m_rotateAngelTouch;
         }
         animation->setStartValue(startvalue);
         animation->setEndValue(endvalue);
        // qDebug()<<"angle finish" <<m_rotateAngelTouch << endvalue;
         connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant &value){
             qreal angle = value.toReal() - m_rotateAngelTouch;
             m_rotateAngelTouch = value.toReal();
             if(value.toReal() != endvalue || m_imgSvgItem)
                this->rotate(angle);
             //setPixmap(pixmap.transformed(t));
         });
        // animation->setLoopCount(1); //旋转次数
         connect(animation, SIGNAL(finished()), this, SLOT(OnFinishPinchAnimal()));
         animation->start(QAbstractAnimation::KeepWhenStopped);
         qDebug() << "finish";

    }
}

void ImageView::swipeTriggered(QSwipeGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        if (gesture->horizontalDirection() == QSwipeGesture::Left ||
                gesture->verticalDirection() == QSwipeGesture::Up) {
            emit nextRequested();
        } else {
            emit previousRequested();
        }
    }
}

void ImageView::OnFinishPinchAnimal()
{
    m_rotateflag = true;
    m_bnextflag = true;
    m_rotateAngelTouch=0;
    if(m_imgSvgItem)
    {
      //  reloadSvgPix(m_path,90,false);
        m_rotateAngel += m_endvalue;
        dApp->m_imageloader->updateImageLoader(QStringList(m_path), true,m_endvalue);
        emit dApp->signalM->sigUpdateThunbnail(m_path);
        return;
    }
    if(!m_pixmapItem) return;
    //QStranform旋转到180度有问题，暂未解决，因此动画结束后旋转Pixmap到180
    QPixmap pixmap;
    pixmap = m_pixmapItem->pixmap();
    QMatrix rotate;
    rotate.rotate(m_endvalue);

    pixmap = pixmap.transformed(rotate, Qt::FastTransformation);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    scene()->clear();
    resetTransform();
    m_pixmapItem = new GraphicsPixmapItem(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    // Make sure item show in center of view after reload
    QRectF rect = m_pixmapItem->boundingRect();
    //            rect.setHeight(rect.height() + 50);
    setSceneRect(rect);
    //            setSceneRect(m_pixmapItem->boundingRect());

    scene()->addItem(m_pixmapItem);
    scale(m_scal,m_scal);
    if(m_bRoate)
    {
        m_rotateAngel += m_endvalue;
        dApp->m_imageloader->updateImageLoader(QStringList(m_path), true,m_endvalue);
        emit dApp->signalM->sigUpdateThunbnail(m_path);
        emit dApp->signalM->UpdateNavImg();
    }
    qDebug() << m_endvalue;
}

#include <QApplication>
void ImageView::showVagueImage(QPixmap thumbnailpixmap,QString filePath)
{
    qDebug() << "sigpath" << filePath;
    m_bStopShowThread=false;
    //一张图片内重复移动不刷新
    if(sigPath == filePath) return;
    sigPath = filePath;
    scene()->clear();
    m_movieItem = nullptr;
    m_imgSvgItem = nullptr;
    resetTransform();
    QRect rect1=  dApp->m_rectmap[filePath];
    //获取主屏幕分辨率lmh0803,如果分辨率大于屏幕分辨率，则采用scaled屏幕分辨率,解决效率问题
    QRect screenRect = QApplication::desktop()->screenGeometry();
    if(rect1.width()>screenRect.width()){
        double dwidth=rect1.width();
        double dheight=rect1.height();
        double witchToheight=dwidth/dheight;
        double hrect=screenRect.width();
        double h=hrect/witchToheight;
        thumbnailpixmap = thumbnailpixmap.scaled(screenRect.width(),static_cast<int>(h));
    }
    else {
            thumbnailpixmap = thumbnailpixmap.scaled(rect1.size());
    }
    m_pixmapItem = new GraphicsPixmapItem(thumbnailpixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    // Make sure item show in center of view after reload
    QRectF rect = m_pixmapItem->boundingRect();

    //            rect.setHeight(rect.height() + 50);
    setSceneRect(rect);
    //            setSceneRect(m_pixmapItem->boundingRect());
    //缩略图显示马赛克较为明显，采用高斯模糊
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(6);
    //blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
    m_pixmapItem->setGraphicsEffect(blurEffect);
    scene()->addItem(m_pixmapItem);

    if ((rect1.width() >= width() || rect1.height() >= height() - 150) && width() > 0 &&
            height() > 0) {
        fitWindow();
    } else {
        fitImage();
    }
    /*lmh0804*/
    emit imageChanged(filePath);
}

void ImageView::showFileImage()
{
    if(m_bStopShowThread) return;
    QVariantList vl = m_watcher.result();
    if (vl.length() == 2) {
        scene()->clear();
        resetTransform();
        m_movieItem = nullptr;
        m_imgSvgItem = nullptr;
        QPixmap pixmap = vl.last().value<QPixmap>();
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        m_pixmapItem = new GraphicsPixmapItem(pixmap);
        m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
        // Make sure item show in center of view after reload
        QRectF rect = m_pixmapItem->boundingRect();
        //            rect.setHeight(rect.height() + 50);
        setSceneRect(rect);
        //            setSceneRect(m_pixmapItem->boundingRect());
        scene()->addItem(m_pixmapItem);
        autoFit();
    }
}

void ImageView::startLoadPixmap()
{
    if(timerPath != sigPath || sigPath.isEmpty())
    {
        timerPath = sigPath;
        showImageFlag = false;
        return;
    }else {
        //在缩略图加载完成前不加载原图
        if((dApp->m_LoadThread && dApp->m_LoadThread->isRunning()) || showImageFlag)
        {
            return;
        }else
        {
            //QImage在线程池中内存泄露更小选用线程池
            if(m_bStopShowThread) return;
            QFuture<QVariantList> f = QtConcurrent::run(m_pool, cacheImage,timerPath);
            if (m_watcher.isRunning()) {
                m_watcher.cancel();
                m_watcher.waitForFinished();
            }
            m_watcher.setFuture(f);
            showImageFlag = true;
        }
    }
}

void ImageView::SlotStopShowThread()
{
    m_bStopShowThread = true;
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    scaleAtPoint(event->pos(), factor);

    event->accept();
    qDebug()<<"21312";
    titleBarControl();
}
