/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Zhang Hao<zhanghao@uniontech.com>
*
* Maintainer: Zhang Hao <zhanghao@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CMANAGERATTRIBUTESERVICE_H
#define CMANAGERATTRIBUTESERVICE_H

#include "module/view/scen/giflib/gif_lib.h"
#include <QObject>
#include <QImage>
#include <QMap>
#include <QTime>
#include <QThread>
/*
* @bref: CManagerAttributeService
*/
class CManagerAttributeService : public QObject
{
    Q_OBJECT
public:
    static CManagerAttributeService *getInstance();
    void setfilePathWithSignalPlay(const QString &path);
    void setfilePath(const QString &path);
    void setCouldRun(bool couldRun);
    inline QImage getImage()
    {
        return m_pImg;
    }
signals:
    void emitImageSignal(QImage image, bool isFirst);
private:
    CManagerAttributeService();
    static CManagerAttributeService *instance;
public:
    int32_t GifLoadFile(void);
    void GifFreeFile(void);//release gif
    int32_t GifFrameShow(void);
    int32_t GifFrameShowSingle(void);
    void GifScreenBufferToRgb888(ColorMapObject *ColorMap, uint8_t *inRgb,
                                 GifRowType *ScreenBuffer, int32_t ScreenWidth, int32_t ScreenHeight,
                                 int alphaIndex = 0);
private:
    GifRecordType gRecordType = UNDEFINED_RECORD_TYPE;
    GifRowType *gpScreenBuffer = nullptr;
    GifFileType *gpGifFile = nullptr;
    QString gEffectGifFile = "";
    QImage m_pImg;
    QImage first;
    int tras;
    bool m_couldRun = true;
    bool m_isFirst = true;
    QTime m_time;
    QThread *m_th = nullptr;
};

#endif // CMANAGERATTRIBUTESERVICE_H
