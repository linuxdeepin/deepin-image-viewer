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
#include "graphicsitem.h"
#include <QMovie>
#include <QDebug>
#include <QPainter>
#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include "application.h"
#include "controller/signalmanager.h"
GraphicsMovieItem::GraphicsMovieItem(const QString &fileName, const QString &suffix, QGraphicsItem *parent)
    : QGraphicsPixmapItem(fileName, parent)
    , gEffectGifFile(fileName)
{
    Q_UNUSED(suffix);
    QFileInfo file(fileName);

    if (file.suffix().contains("gif")) {
        m_index = 0;
        gEffectGifFile = fileName;
        QObject::connect(dApp->signalM, &SignalManager::sigGifImageRe, this, [=] {
            setPixmap(QPixmap::fromImage(first));
        });
        QObject::connect(dApp, &Application::endThread, this, [=]() {
            m_bRetThread = false;
            if (nullptr != m_th) {
                m_th->wait();
            }
        });
        m_th = QThread::create([=]() {
            while (m_bRetThread) {
                GifLoadFile();
                GifFrameShow();
                GifFreeFile();
            }

        });

        m_th->start();
        //        m_pTImer = new QTimer(this);

        //        QObject::connect(m_pTImer, &QTimer::timeout, this, [=] {
        //                        setPixmap(QPixmap::fromImage(first));
        //        });
        //        m_pTImer->start(50);
    } else {
        m_movie = new QMovie(fileName);
        QObject::connect(m_movie, &QMovie::frameChanged, this, [=] {
            if (m_movie.isNull())
                return;
            setPixmap(m_movie->currentPixmap());
        });
    }
}

GraphicsMovieItem::~GraphicsMovieItem()
{
    // Prepares the item for a geometry change. Call this function
    // before changing the bounding rect of an item to keep
    // QGraphicsScene's index up to date.
    // If not doing this, it may crash
    prepareGeometryChange();

    m_bRetThread = false;
    if (nullptr != m_th) {
        m_th->wait();
    }

    if (nullptr != m_movie) {
        m_movie->stop();
        m_movie->deleteLater();
        m_movie = nullptr;
    }
}

/*!
 * \brief GraphicsMovieItem::isValid
 * There is a bug with QMovie::isValid() that is event if file's format not
 * supported this function still return true.
 * \return
 */
bool GraphicsMovieItem::isValid() const
{
    return m_movie->frameCount() > 1;
}

void GraphicsMovieItem::start()
{
    if (nullptr != m_movie) {
        m_movie->start();
    }
}

void GraphicsMovieItem::stop()
{
    if (nullptr != m_movie) {
        m_movie->stop();
    }
}


GraphicsPixmapItem::GraphicsPixmapItem(const QPixmap &pixmap)
    : QGraphicsPixmapItem(pixmap, nullptr)
{

}

GraphicsPixmapItem::~GraphicsPixmapItem()
{
    prepareGeometryChange();
}

void GraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QTransform ts = painter->transform();

    if (ts.type() == QTransform::TxScale && ts.m11() < 1) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform,
                               (transformationMode() == Qt::SmoothTransformation));

        QPixmap pixmap;

        if (qIsNull(cachePixmap.first - ts.m11())) {
            pixmap = cachePixmap.second;
        } else {
            pixmap = this->pixmap().transformed(painter->transform(), transformationMode());
            cachePixmap = qMakePair(ts.m11(), pixmap);
        }

        pixmap.setDevicePixelRatio(painter->device()->devicePixelRatioF());
        painter->resetTransform();
        painter->drawPixmap(offset() + QPointF(ts.dx(), ts.dy()), pixmap);
        painter->setTransform(ts);
    } else {
        QGraphicsPixmapItem::paint(painter, option, widget);
    }
}

int32_t GraphicsMovieItem::GifLoadFile()
{
    int32_t error = 0;
    int32_t size = 0;
    int32_t idx = 0;
    int32_t ret = 0;

    do {
        if (!m_bRetThread) {
            return -1;
        }
        if (nullptr == gEffectGifFile) {
            ret = -1;
            break;
        }

        gpGifFile = DGifOpenFileName(gEffectGifFile.toStdString().c_str(), &error);
        if (nullptr == gpGifFile) {
            ret = -2;
            break;
        }

        if ((gpGifFile->SHeight == 0) || (gpGifFile->SWidth == 0)) {
            ret = -3;
            break;
        }

        gpScreenBuffer = static_cast<GifRowType *>(malloc(static_cast<size_t>(gpGifFile->SHeight * static_cast<int32_t>((sizeof(GifRowType))))));
        if (nullptr == gpScreenBuffer) {
            ret = -4;
            break;
        }

        /* Size in bytes one row */
        size = gpGifFile->SWidth * static_cast<int32_t>(sizeof(GifPixelType));
        gpScreenBuffer[0] = static_cast<GifRowType>(malloc(static_cast<size_t>(size)));
        if (nullptr == gpScreenBuffer[0]) {
            ret = -5;
            break;
        }

        /* Set its color to BackGround */
        for (idx = 0; idx < gpGifFile->SWidth; idx++) {
            gpScreenBuffer[0][idx] = static_cast<unsigned char>(gpGifFile->SBackGroundColor);
        }

        /* Allocate the other rows, and set their color to background too */
        for (idx = 1; idx < gpGifFile->SHeight; idx++) {
            gpScreenBuffer[idx] = static_cast<GifRowType>(malloc(static_cast<size_t>(size)));
            if (nullptr == gpScreenBuffer[idx]) {
                ret = -6;
                break;
            }
            memcpy(gpScreenBuffer[idx], gpScreenBuffer[0], static_cast<size_t>(size));
        }

        if (0 > ret) {
            break;
        }
    } while (0);

    if (0 > ret) {
        GifFreeFile();
    }

    return ret;
}

void GraphicsMovieItem::GifFreeFile()
{
    int32_t idx = 0;
    int32_t error = 0;
    if (!m_bRetThread) {
        return;
    }
    for (idx = 0; idx < gpGifFile->SHeight; idx++) {
        if (nullptr != gpScreenBuffer[idx]) {
            free(gpScreenBuffer[idx]);
            gpScreenBuffer[idx] = nullptr;
        }
    }

    if (nullptr != gpScreenBuffer) {
        free(gpScreenBuffer);
        gpScreenBuffer = nullptr;
    }

    if (nullptr != gpGifFile) {
        DGifCloseFile(gpGifFile, &error);
        gpGifFile = nullptr;
    }
}

int32_t GraphicsMovieItem::GifFrameShow()
{
    ColorMapObject *colorMap = nullptr;
    GifByteType *extension = nullptr;

    int32_t InterlacedOffset[] = {0, 4, 2, 1}; // The way Interlaced image should
    int32_t InterlacedJumps[] = {8, 8, 4, 2}; // be read - offsets and jumps...
    uint8_t rgbBuf[240 * 320] = {0};
    int32_t extCode = 0;
    int32_t row = 0;
    int32_t col = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t iW = 0;
    int32_t iH = 0;
    int32_t ret = 0;
    static int temp = 0;
    do {
        if (!m_bRetThread) {
            return -1;
        }
        if (DGifGetRecordType(gpGifFile, &gRecordType) == GIF_ERROR) {
            ret = -1;
            break;
        }

        switch (gRecordType) {
        case IMAGE_DESC_RECORD_TYPE: {
            if (DGifGetImageDesc(gpGifFile) == GIF_ERROR) {
                ret = -2;
                break;
            }

            row = gpGifFile->Image.Top;
            col = gpGifFile->Image.Left;
            width = gpGifFile->Image.Width;
            height = gpGifFile->Image.Height;

            if (gpGifFile->Image.Interlace) {
                for (iH = 0; iH < 4; iH++) {
                    for (iW = row + InterlacedOffset[iH]; iW < row + height; iW += InterlacedJumps[iH]) {
                        DGifGetLine(gpGifFile, &gpScreenBuffer[iW][col], width);
                    }
                }
            } else {
                for (iH = 0; iH < height; iH++) {
                    DGifGetLine(gpGifFile, &gpScreenBuffer[row++][col], width);
                }
            }

            colorMap = (gpGifFile->Image.ColorMap ? gpGifFile->Image.ColorMap : gpGifFile->SColorMap);
            if (colorMap == nullptr) {
                ret = -3;
                break;
            }
            GifScreenBufferToRgb888(colorMap, rgbBuf, gpScreenBuffer,
                                    gpGifFile->SWidth, gpGifFile->SHeight, tras);
            break;
        }
        case EXTENSION_RECORD_TYPE:

            /* Skip any extension blocks in file: */
            if (DGifGetExtension(gpGifFile, &extCode, &extension) == GIF_ERROR) {
                ret = -4;
                break;
            }
            if (extension != nullptr) {
                if (extension[0] & 0x01)
                    tras = NO_TRANSPARENT_COLOR;
                else
                    tras = static_cast<int>(extension[4]);
            }

            while (extension != nullptr) {
                temp++;
                if (DGifGetExtensionNext(gpGifFile, &extension) == GIF_ERROR) {
                    ret = -5;
                    break;
                }
                if (extension != nullptr) {
                    if (extension[0] & 0x01)
                        tras = NO_TRANSPARENT_COLOR;
                    else
                        tras = static_cast<int>(extension[4]);
                }
            }

            break;

        case TERMINATE_RECORD_TYPE:
            break;

        default:
            break;
        }

        if (0 < ret) {
            break;
        }
    } while (gRecordType != TERMINATE_RECORD_TYPE);

    return ret;
}
#include <QByteArray>
void GraphicsMovieItem::GifScreenBufferToRgb888(ColorMapObject *ColorMap,
                                                uint8_t *inRgb,
                                                GifRowType *ScreenBuffer,
                                                int32_t ScreenWidth,
                                                int32_t ScreenHeight,
                                                int alphaIndex)
{
    Q_UNUSED(inRgb);
    if (m_fisrtImage) {
        first = QImage(ScreenWidth, ScreenHeight, QImage::Format_RGB32);
        m_fisrtImage = false;
    }
    GifColorType *ColorMapEntry = nullptr;
    // GifRowType GifRow = nullptr;
    QByteArray byte;
    int32_t idxH = 0;
    int32_t idxW = 0;
    int startTime = static_cast<int>(QDateTime::currentMSecsSinceEpoch());
    // QImage img(ScreenWidth, ScreenHeight, QImage::Format_ARGB32);
    for (idxH = 0; idxH < ScreenHeight; idxH++) {

        for (idxW = 0; idxW < ScreenWidth; idxW++) {
            ColorMapEntry = &ColorMap->Colors[ScreenBuffer[idxH][idxW]];
            //如果是透明色
            if (alphaIndex == ScreenBuffer[idxH][idxW] || first.pixel(idxW, idxH) == qRgb(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue)) {
                //img.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 0));
            } else {
                // img.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 255));
                first.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 255));
            }
        }
    }

    if (!m_bRetThread) {
        return;
    }

    int endTime = static_cast<int>(QDateTime::currentMSecsSinceEpoch());
    int tolTime = endTime - startTime;

    if (tolTime < 100) {
        QThread::msleep(static_cast<unsigned long>((100 - tolTime)));
    }

    emit dApp->signalM->sigGifImageRe();
}
