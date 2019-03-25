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

class QMovie;
class GraphicsMovieItem : public QGraphicsPixmapItem, QObject
{
public:
    explicit GraphicsMovieItem(const QString &fileName, QGraphicsItem *parent = 0);
    ~GraphicsMovieItem();
    bool isValid() const;
    void start();
    void stop();

private:
    QPointer<QMovie> m_movie;
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
