#include "thumbnaildelegate.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "utils/imageutils.h"
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>

namespace {

const int BORDER_RADIUS = 0;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;
const QColor BORDER_COLOR = QColor(255, 255, 255, 35);
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");

const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}

ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void ThumbnailDelegate::clearPaintingList()
{
    m_paintingPaths.clear();
}

const QStringList ThumbnailDelegate::paintingPaths() const
{
    return m_paintingPaths;
}

void ThumbnailDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    const ItemData data = itemData(index);
    if (data.path.isEmpty()) return;

    bool selected = false;
    if (/*(option.state & QStyle::State_MouseOver) &&*/
            (option.state & QStyle::State_Selected) != 0) {
        selected = true;
    }

    painter->setRenderHints(QPainter::HighQualityAntialiasing |
                            QPainter::SmoothPixmapTransform |
                            QPainter::Antialiasing);
    QRect rect = option.rect;
    QPainterPath bp;
    bp.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);
    painter->setClipPath(bp);
    QPixmap thumb;
    using namespace utils::image;
    // When a thumbnail is changed externally, is no longer used in the data model
    if (! thumbnailExist(data.path) || data.thumbnail.isNull()) {
        // Cache thumbnail not exist but model's thumbnail still exist, use the model one
        if (! data.thumbnail.isNull()) {
            thumb = data.thumbnail;
        }
        else if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
            const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
            thumb = cutSquareImage(QPixmap(
                ":/images/resources/images/default_thumbnail.png"), ms);
            QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumb);
        }

        // Generate thumbnail and update data from view
        if (m_paintingPaths.indexOf(data.path) == -1) {
            m_paintingPaths << data.path;
        }
    }
    else {
        thumb = data.thumbnail;
    }
    painter->drawPixmap(rect, thumb);

    // Draw inside border
    QPen p(selected ? BORDER_COLOR_SELECTED : BORDER_COLOR,
           selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
    painter->setPen(p);
    QPainterPathStroker stroker;
    stroker.setWidth(selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath borderPath = stroker.createStroke(bp);
    painter->drawPath(borderPath);
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.model()->data(index, Qt::SizeHintRole).toSize();
}

ThumbnailDelegate::ItemData ThumbnailDelegate::itemData(const QModelIndex &index) const
{
    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
    ItemData data;
    if (datas.length() >= 1) {
        data.name = datas[0].toString();
    }
    if (datas.length() >= 2) {
        data.path = datas[1].toString();
    }
    if (datas.length() >= 3) {
        data.thumbnail.loadFromData(datas[2].toByteArray());
    }

    return data;
}

