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
#ifndef IMAGESVGITEM_H
#define IMAGESVGITEM_H

#include <QGraphicsObject>

#include <DSvgRenderer>
#include <QRectF>

DGUI_USE_NAMESPACE

#include <QtCore/qglobal.h>

#if !defined(QT_NO_GRAPHICSVIEW) && !defined(QT_NO_WIDGETS)

#include <QtWidgets/qgraphicsitem.h>

#include <QtSvg/qtsvgglobal.h>
#include <QGraphicsSvgItem>
class QSvgRenderer;
class ImageSvgItemPrivate;

class ImageSvgItem : public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(QString elementId READ elementId WRITE setElementId)
    Q_PROPERTY(QSize maximumCacheSize READ maximumCacheSize WRITE setMaximumCacheSize)

public:
    ImageSvgItem(QGraphicsItem *parentItem = nullptr);
    ImageSvgItem(const QString &fileName, QGraphicsItem *parentItem = nullptr);
    ~ImageSvgItem() override;

    void setSharedRenderer(QSvgRenderer *renderer);
    QSvgRenderer *renderer() const;

    void setElementId(const QString &id);
    QString elementId() const;

    void setCachingEnabled(bool);
    bool isCachingEnabled() const;

    void setMaximumCacheSize(const QSize &size);
    QSize maximumCacheSize() const;

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    enum { Type = 13 };
    int type() const override;

    void updateDefaultSize();
private:

    QSvgRenderer *m_renderer = nullptr;
    QRectF m_boundingRect;
    QString m_elemId;

//    Q_PRIVATE_SLOT(d_func(), void _q_repaintItem())
};

#endif // QT_NO_GRAPHICSVIEW or QT_NO_WIDGETS

#endif // IMAGESVGITEM_H
