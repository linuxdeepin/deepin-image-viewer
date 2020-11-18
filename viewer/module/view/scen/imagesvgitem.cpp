//#include "imagesvgitem.h"

//#if !defined(QT_NO_GRAPHICSVIEW) && !defined(QT_NO_WIDGETS)

//#include "dsvgrenderer.h"
//#include "qdebug.h"
//#include "qpainter.h"
//#include "qstyleoption.h"

//#include "private/qgraphicsitem_p.h"
//#include "private/qobject_p.h"
//#include <QSvgRenderer>
//QT_BEGIN_NAMESPACE

//#define Q_DECLARE_PUBLIC(Class)                                    \
//    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
//    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
//    friend class Class;


//class ImageSvgItemPrivate : public QGraphicsItemPrivate
//{
//public:
//    Q_DECLARE_PUBLIC(ImageSvgItem)

//    ImageSvgItemPrivate()
//        : renderer(nullptr)
//        , shared(false)
//    {
//    }

//    void init(QGraphicsItem *parent)
//    {
//        Q_Q(ImageSvgItem);
//        q->setParentItem(parent);
//        renderer = new QSvgRenderer(q);
//        //        QObject::connect(renderer, SIGNAL(repaintNeeded()),
//        //                         q, SLOT(_q_repaintItem()));
//        q->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
//        q->setMaximumCacheSize(QSize(1024, 768));
//    }

//    //    void _q_repaintItem()
//    //    {
//    //        q_func()->update();
//    //    }

//    inline void updateDefaultSize()
//    {
//        QRectF bounds;
//        if (elemId.isEmpty()) {
//            bounds = QRectF(QPointF(0, 0), renderer->defaultSize());
//        } else {
//            bounds = renderer->boundsOnElement(elemId);
//        }
//        if (boundingRect.size() != bounds.size()) {
//            q_func()->prepareGeometryChange();
//            boundingRect.setSize(bounds.size());
//        }
//    }

//    QSvgRenderer *renderer;
//    QRectF boundingRect;
//    bool shared;
//    QString elemId;
//};

//ImageSvgItem::ImageSvgItem(QGraphicsItem *parent)
////    :QGraphicsSvgItem(parent)
//    : QGraphicsObject(*new ImageSvgItemPrivate(), 0)
//{
//    Q_D(ImageSvgItem);
//    d->init(parent);
//}

//ImageSvgItem::ImageSvgItem(const QString &fileName, QGraphicsItem *parent)
////:QGraphicsSvgItem(parent)
//    : QGraphicsObject(*new ImageSvgItemPrivate(), 0)
//{
//    Q_D(ImageSvgItem);
//    d->init(parent);
//    d->renderer->load(fileName);
//    d->updateDefaultSize();
//}

//QSvgRenderer *ImageSvgItem::renderer() const
//{
//    return d_func()->renderer;
//}

//QRectF ImageSvgItem::boundingRect() const
//{
//    Q_D(const ImageSvgItem);
//    return d->boundingRect;
//}

//static void qt_graphicsItem_highlightSelected(QGraphicsItem *item, QPainter *painter,
//                                              const QStyleOptionGraphicsItem *option)
//{
//    const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
//    if (qFuzzyIsNull(qMax(murect.width(), murect.height())))
//        return;

//    const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
//    if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
//        return;

//    qreal itemPenWidth;
//    switch (item->type()) {
//        case QGraphicsEllipseItem::Type:
//            itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
//            break;
//        case QGraphicsPathItem::Type:
//            itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
//            break;
//        case QGraphicsPolygonItem::Type:
//            itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
//            break;
//        case QGraphicsRectItem::Type:
//            itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
//            break;
//        case QGraphicsSimpleTextItem::Type:
//            itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
//            break;
//        case QGraphicsLineItem::Type:
//            itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
//            break;
//        default:
//            itemPenWidth = 1.0;
//    }
//    const qreal pad = itemPenWidth / 2;

//    const qreal penWidth = 0;  // cosmetic pen

//    const QColor fgcolor = option->palette.windowText().color();
//    const QColor bgcolor(  // ensure good contrast against fgcolor
//        fgcolor.red() > 127 ? 0 : 255, fgcolor.green() > 127 ? 0 : 255,
//        fgcolor.blue() > 127 ? 0 : 255);

//    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
//    painter->setBrush(Qt::NoBrush);
//    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

//    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
//    painter->setBrush(Qt::NoBrush);
//    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
//}

//void ImageSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
//{
//    //    Q_UNUSED(option);
//    Q_UNUSED(widget);

//    Q_D(ImageSvgItem);
//    if (!d->renderer->isValid())
//        return;

//    if (d->elemId.isEmpty())
//        d->renderer->render(painter, d->boundingRect);
//    else
//        d->renderer->render(painter, d->elemId, d->boundingRect);

//    if (option->state & QStyle::State_Selected)
//        qt_graphicsItem_highlightSelected(this, painter, option);
//}

//int ImageSvgItem::type() const
//{
//    return Type;
//}

//void ImageSvgItem::setMaximumCacheSize(const QSize &size)
//{
//    QGraphicsItem::d_ptr->setExtra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize, size);
//    update();
//}

//QSize ImageSvgItem::maximumCacheSize() const
//{
//    return QGraphicsItem::d_ptr->extra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize).toSize();
//}

//void ImageSvgItem::setElementId(const QString &id)
//{
//    Q_D(ImageSvgItem);
//    d->elemId = id;
//    d->updateDefaultSize();
//    update();
//}

//QString ImageSvgItem::elementId() const
//{
//    Q_D(const ImageSvgItem);
//    return d->elemId;
//}

//void ImageSvgItem::setSharedRenderer(QSvgRenderer *renderer)
//{
//    Q_D(ImageSvgItem);
//    if (!d->shared)
//        delete d->renderer;

//    d->renderer = renderer;
//    d->shared = true;

//    d->updateDefaultSize();

//    update();
//}

//void ImageSvgItem::setCachingEnabled(bool caching)
//{
//    setCacheMode(caching ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);
//}

//bool ImageSvgItem::isCachingEnabled() const
//{
//    return cacheMode() != QGraphicsItem::NoCache;
//}

//#endif  // QT_NO_WIDGETS
