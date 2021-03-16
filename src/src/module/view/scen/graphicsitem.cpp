/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
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
#include "graphicsitem.h"

#include <QDebug>
#include <QPainter>

GraphicsMovieItem::GraphicsMovieItem(const QString &fileName, const QString &suffix, QGraphicsItem *parent)
    : QGraphicsPixmapItem(fileName, parent)
{
    Q_UNUSED(suffix);
    m_movie = new QMovie(fileName);
    QObject::connect(m_movie, &QMovie::frameChanged, this, [ = ] {
        if (m_movie.isNull()) return;
        setPixmap(m_movie->currentPixmap());
    });
}

GraphicsMovieItem::~GraphicsMovieItem()
{
    // Prepares the item for a geometry change. Call this function
    // before changing the bounding rect of an item to keep
    // QGraphicsScene's index up to date.
    // If not doing this, it may crash
    prepareGeometryChange();

    m_movie->stop();
    m_movie->deleteLater();
    m_movie = nullptr;
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
    m_movie->start();
}

void GraphicsMovieItem::stop()
{
    m_movie->stop();
}


GraphicsPixmapItem::GraphicsPixmapItem(const QPixmap &pixmap)
    : QGraphicsPixmapItem(pixmap, nullptr)
{

}

GraphicsPixmapItem::~GraphicsPixmapItem()
{
    prepareGeometryChange();
}

void GraphicsPixmapItem::setPixmap(const QPixmap &pixmap)
{
    cachePixmap = qMakePair(cachePixmap.first, pixmap);
    QGraphicsPixmapItem::setPixmap(pixmap);
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



