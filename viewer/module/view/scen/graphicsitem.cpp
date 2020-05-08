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
#include "utils/imageutils.h"
GraphicsMovieItem::GraphicsMovieItem(const QString &fileName, QGraphicsItem *parent)
    : QGraphicsPixmapItem(fileName, parent)
    , m_index(0)
{
    //用freeimage解析gif
    m_pGif = utils::image::openGiffromPath(fileName);

    m_pTImer = new QTimer(this);
    QObject::connect(m_pTImer, &QTimer::timeout, this, [ = ] {
        //用freeimage解析的图片显示
        setPixmap(QPixmap::fromImage(utils::image::getGifImage(m_index, m_pGif)));
        m_index++;
        if (m_index >= utils::image::getGifImageCount(m_pGif))
        {
            m_index = 0;
        }
    });
    m_pTImer->start(100);
//    QObject::connect(m_movie, &QMovie::frameChanged, this, [ = ](int index) {
//        if (m_movie.isNull()) return;
//        //setPixmap(m_movie->currentPixmap());
//        //用freeimage解析的图片显示
//        setPixmap(QPixmap::fromImage(utils::image::getGifImage(index, m_pGif)));
//    });
}

GraphicsMovieItem::~GraphicsMovieItem()
{
    // Prepares the item for a geometry change. Call this function
    // before changing the bounding rect of an item to keep
    // QGraphicsScene's index up to date.
    // If not doing this, it may crash
    prepareGeometryChange();
    m_pTImer->stop();
    m_pTImer->deleteLater();
    m_pTImer = nullptr;

//    m_movie->stop();
//    m_movie->deleteLater();
//    m_movie = nullptr;
}

/*!
 * \brief GraphicsMovieItem::isValid
 * There is a bug with QMovie::isValid() that is event if file's format not
 * supported this function still return true.
 * \return
 */
bool GraphicsMovieItem::isValid() const
{
    return utils::image::getGifImageCount(m_pGif) > 1;
}

void GraphicsMovieItem::start()
{
    //m_movie->start();
    m_pTImer->start(100);
}

void GraphicsMovieItem::stop()
{
    //m_movie->stop();
    m_pTImer->stop();
}


GraphicsPixmapItem::GraphicsPixmapItem(const QPixmap &pixmap)
    : QGraphicsPixmapItem(pixmap, NULL)
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
