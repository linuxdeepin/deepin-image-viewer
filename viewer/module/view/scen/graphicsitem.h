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
#ifndef GRAPHICSMOVIEITEM_H
#define GRAPHICSMOVIEITEM_H

#include <QGraphicsPixmapItem>
#include <QPointer>
#include <gif_lib.h>
class QMovie;
class GraphicsMovieItem : public QGraphicsPixmapItem, QObject
{
public:
    explicit GraphicsMovieItem(const QString &fileName,const QString &suffix=NULL,QGraphicsItem *parent = 0);
    ~GraphicsMovieItem();
    bool isValid() const;
    void start();
    void stop();
    /**
     * @brief GifLoadFile
     * 加载gif文件，分配资源
     * @return int32_t
     */
    int32_t GifLoadFile(void);
    /**
     * @brief GifLoadFile
     * 释放gif文件，释放资源
     * @return
     */
    void GifFreeFile(void);
    /**
     * @brief GifFrameShow
     * 读取gif图片
     * @return
     */
    int32_t GifFrameShow(void);
    /**
     * @brief GifScreenBufferToRgb888
     * 读取buffer转化为rgb，逐个对点绘制
     * @return
     */
    void GifScreenBufferToRgb888(ColorMapObject *ColorMap, uint8_t *inRgb,
                                 GifRowType *ScreenBuffer, int32_t ScreenWidth, int32_t ScreenHeight,
                                 int alphaIndex = 0);

private:
    QPointer<QMovie> m_movie;

    GifRecordType gRecordType = UNDEFINED_RECORD_TYPE;
    GifRowType *gpScreenBuffer = nullptr;
    GifFileType *gpGifFile = nullptr;
    QString gEffectGifFile;

    QMap<int, QImage> m_map;
    QImage first;
    QImage m_currentImage;
    int tras;
    int m_index = 0;
    int m_indexDisplay = 0;
    QString m_suffix;
    QTimer *m_pTImer = nullptr;
    bool m_bRetThread = true;

    QThread *m_th;
};

class GraphicsPixmapItem : public QGraphicsPixmapItem
{
public:
    explicit GraphicsPixmapItem(const QPixmap &pixmap);
    ~GraphicsPixmapItem();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPair<qreal, QPixmap> cachePixmap;
};

#endif // GRAPHICSMOVIEITEM_H
