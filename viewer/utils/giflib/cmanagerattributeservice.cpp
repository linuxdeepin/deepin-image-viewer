/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     Iceyer<Iceyer@uniontech.com>
* Maintainer: Iceyer <Iceyer@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "cmanagerattributeservice.h"
#include <stdlib.h>
#include "malloc.h"
#include <QGraphicsItem>
#include <QDebug>
#include <QtConcurrent>

CManagerAttributeService *CManagerAttributeService::instance = nullptr;
CManagerAttributeService *CManagerAttributeService::getInstance()
{
    if (nullptr == instance) {
        instance = new CManagerAttributeService();
    }
    return instance;
}

void CManagerAttributeService::setfilePathWithSignalPlay(const QString &path)
{
    m_couldRun = true;

    m_th = QThread::create([ = ]() {
        while (m_couldRun) {
            m_isFirst = true;
            gEffectGifFile = path;
            int32_t ret = GifLoadFile();
            if (ret >= 0) {
                GifFrameShow();
            }

        }
        GifFreeFile();
    });

    m_th->start();
}

void CManagerAttributeService::setfilePath(const QString &path)
{
    m_couldRun = true;
    m_isFirst = true;
    gEffectGifFile = path;
    GifLoadFile();
}

void CManagerAttributeService::setCouldRun(bool couldRun)
{
    m_couldRun = couldRun;
    if (nullptr != m_th) {
        m_th->quit();
        m_th->wait();
    }
}

int32_t CManagerAttributeService::GifLoadFile()
{
    int32_t error = 0;
    int32_t size = 0;
    int32_t idx = 0;
    int32_t ret = 0;

    do {
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

        gpScreenBuffer = (GifRowType *)malloc(gpGifFile->SHeight * sizeof(GifRowType));
        if (nullptr == gpScreenBuffer) {
            ret = -4;
            break;
        }

        /* Size in bytes one row */
        size = gpGifFile->SWidth * sizeof(GifPixelType);
        gpScreenBuffer[0] = (GifRowType)malloc(size);
        if (nullptr == gpScreenBuffer[0]) {
            ret = -5;
            break;
        }

        /* Set its color to BackGround */
        for (idx = 0; idx < gpGifFile->SWidth; idx++) {
            gpScreenBuffer[0][idx] = gpGifFile->SBackGroundColor;
        }

        /* Allocate the other rows, and set their color to background too */
        for (idx = 1; idx < gpGifFile->SHeight; idx++) {
            gpScreenBuffer[idx] = (GifRowType)malloc(size);
            if (nullptr == gpScreenBuffer[idx]) {
                ret = -6;
                break;
            }
            memcpy(gpScreenBuffer[idx], gpScreenBuffer[0], size);
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

void CManagerAttributeService::GifFreeFile()
{
    int32_t idx = 0;
    int32_t error = 0;
    if (nullptr != gpGifFile) {
        for (idx = 0; idx < gpGifFile->SHeight; idx++) {
            if (nullptr != gpScreenBuffer[idx]) {
                free(gpScreenBuffer[idx]);
                gpScreenBuffer[idx] = nullptr;
            }
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


int32_t CManagerAttributeService::GifFrameShow()
{
    ColorMapObject *colorMap = nullptr;
    GifByteType *extension = nullptr;

    int32_t InterlacedOffset[] = { 0, 4, 2, 1 };  // The way Interlaced image should
    int32_t InterlacedJumps[] = { 8, 8, 4, 2 };   // be read - offsets and jumps...
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
        if (!m_couldRun) {
            break;
        }
        if (DGifGetRecordType(gpGifFile, &gRecordType) == GIF_ERROR) {
            ret = -1;
            break;
        }

        switch (gRecordType) {
        case IMAGE_DESC_RECORD_TYPE: {
            if (!m_couldRun) {
                break;
            }
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

            colorMap = (gpGifFile->Image.ColorMap ?
                        gpGifFile->Image.ColorMap : gpGifFile->SColorMap);
            if (colorMap == nullptr) {
                ret = -3;
                break;
            }

            GifScreenBufferToRgb888(colorMap, rgbBuf, gpScreenBuffer,
                                    gpGifFile->SWidth, gpGifFile->SHeight, tras);
            //QImage img((uchar *)rgbBuf, gpGifFile->SWidth, gpGifFile->SHeight, QImage::Format_RGB32);
            break;
        }
        case EXTENSION_RECORD_TYPE: {
            if (!m_couldRun) {
                break;
            }
            /* Skip any extension blocks in file: */
            if (DGifGetExtension(gpGifFile, &extCode, &extension) == GIF_ERROR) {
                ret = -4;
                break;
            }
            if (extension != nullptr) {
                if (extension[0] & 0x01)
                    tras = NO_TRANSPARENT_COLOR;
                else
                    tras = (int)extension[4];
            }

            while (extension != NULL) {
                if (!m_couldRun) {
                    break;
                }
                if (DGifGetExtensionNext(gpGifFile, &extension) == GIF_ERROR) {
                    ret = -5;
                    break;
                }
                if (extension != nullptr) {
                    if (extension[0] & 0x01)
                        tras = NO_TRANSPARENT_COLOR;
                    else
                        tras = (int)extension[4];
                }
            }
            break;
        }
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

int32_t CManagerAttributeService::GifFrameShowSingle()
{
    ColorMapObject *colorMap = nullptr;
    GifByteType *extension = nullptr;
    int32_t InterlacedOffset[] = { 0, 4, 2, 1 };  // The way Interlaced image should
    int32_t InterlacedJumps[] = { 8, 8, 4, 2 };   // be read - offsets and jumps...
    uint8_t rgbBuf[240 * 320] = {0};
    int32_t extCode = 0;
    int32_t row = 0;
    int32_t col = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t iW = 0;
    int32_t iH = 0;
    int32_t ret = 0;
    if (gRecordType != TERMINATE_RECORD_TYPE) {
        switch (gRecordType) {
        case IMAGE_DESC_RECORD_TYPE: {
            if (!m_couldRun) {
                break;
            }
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
            colorMap = (gpGifFile->Image.ColorMap ?
                        gpGifFile->Image.ColorMap : gpGifFile->SColorMap);
            if (colorMap == nullptr) {
                ret = -3;
                break;
            }
            GifScreenBufferToRgb888(colorMap, rgbBuf, gpScreenBuffer,
                                    gpGifFile->SWidth, gpGifFile->SHeight, tras);
            //QImage img((uchar *)rgbBuf, gpGifFile->SWidth, gpGifFile->SHeight, QImage::Format_RGB32);
            break;
        }
        case EXTENSION_RECORD_TYPE: {
            if (!m_couldRun) {
                break;
            }
            /* Skip any extension blocks in file: */
            if (DGifGetExtension(gpGifFile, &extCode, &extension) == GIF_ERROR) {
                ret = -4;
                break;
            }
            if (extension != nullptr) {
                if (extension[0] & 0x01)
                    tras = NO_TRANSPARENT_COLOR;
                else
                    tras = (int)extension[4];
            }
            while (extension != NULL) {
                if (!m_couldRun) {
                    break;
                }
                if (DGifGetExtensionNext(gpGifFile, &extension) == GIF_ERROR) {
                    ret = -5;
                    break;
                }
                if (extension != nullptr) {
                    if (extension[0] & 0x01)
                        tras = NO_TRANSPARENT_COLOR;
                    else
                        tras = (int)extension[4];
                }

            }
            break;
        }
        case TERMINATE_RECORD_TYPE:
            break;

        default:
            break;
        }

        if (0 < ret) {
            return 0;
        }
    }
    return 0;
}

void CManagerAttributeService::GifScreenBufferToRgb888(ColorMapObject *ColorMap,
                                                       uint8_t *inRgb,
                                                       GifRowType *ScreenBuffer,
                                                       int32_t ScreenWidth,
                                                       int32_t ScreenHeight,
                                                       int alphaIndex)
{
    Q_UNUSED(inRgb);
    if (m_isFirst) {
        first = QImage(ScreenWidth, ScreenHeight, QImage::Format_RGB32);
        m_isFirst = false;
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

    if (!m_couldRun) {
        return;
    }

    int endTime = static_cast<int>(QDateTime::currentMSecsSinceEpoch());
    int tolTime = endTime - startTime;

    if (tolTime < 100) {
        QThread::msleep(static_cast<unsigned long>((100 - tolTime)));
    }

    emit emitImageSignal(first, m_isFirst);
}

//void CManagerAttributeService::GifScreenBufferToRgb888(ColorMapObject *ColorMap, uint8_t *inRgb, GifRowType *ScreenBuffer, int32_t ScreenWidth, int32_t ScreenHeight, int alphaIndex)
//{
//    GifColorType *ColorMapEntry = nullptr;
//    GifRowType GifRow = nullptr;
//    uint8_t *rgbBuf = inRgb;
//    int32_t idxH = 0;
//    int32_t idxW = 0;
//    m_pImg = QImage(ScreenWidth, ScreenHeight, QImage::Format_ARGB32);
//    m_time.restart();
//    for (idxH = 0; idxH < ScreenHeight; idxH++) {

//        for (idxW = 0; idxW < ScreenWidth; idxW++) {
//            ColorMapEntry = &ColorMap->Colors[ScreenBuffer[idxH][idxW]];
//            //如果是透明色
//            if (alphaIndex == ScreenBuffer[idxH][idxW] || first.pixel(idxW, idxH) == qRgb(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue)) {
//                //img.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 0));
//            } else {
//                // img.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 255));
//                m_pImg.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 255));
//            }
//        }
//    }
//    int temp = m_time.elapsed();
//    //emit emitImageSignal(img, m_isFirst);
//    if (temp >= 100) {
//        emit emitImageSignal(m_pImg, m_isFirst);
//    } else {
//        QThread::msleep(100 - temp);
//        emit emitImageSignal(m_pImg, m_isFirst);
//    }

//    m_isFirst = false;
//    //m_map.insert(m_index++, img);
//}

CManagerAttributeService::CManagerAttributeService()
{
}


