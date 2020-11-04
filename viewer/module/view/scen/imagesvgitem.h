#ifndef IMAGESVGITEM_H
#define IMAGESVGITEM_H

#include <QGraphicsObject>

#include <DSvgRenderer>

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

private:
    Q_DISABLE_COPY(ImageSvgItem)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), ImageSvgItem)

//    Q_PRIVATE_SLOT(d_func(), void _q_repaintItem())
};

#endif // QT_NO_GRAPHICSVIEW or QT_NO_WIDGETS

#endif // IMAGESVGITEM_H
