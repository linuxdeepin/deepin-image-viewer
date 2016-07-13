#include "thumbnaildelegate.h"
#include "utils/imageutils.h"
#include "controller/databasemanager.h"
#include <QDateTime>
#include <QLineEdit>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QHBoxLayout>
#include <QPixmapCache>

namespace {

const int BORDER_RADIUS = 4;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;
const QColor BORDER_COLOR = QColor(255, 255, 255, 51);
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");

const int TICKED_MARK_SIZE = 24;

const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}

ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void ThumbnailDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        const QString name = datas[0].toString();
//        const QString path = datas[1].toString();
        const bool tickable = datas[2].toBool();
        bool selected = false;
        if (/*(option.state & QStyle::State_MouseOver) &&*/
                (option.state & QStyle::State_Selected) != 0) {
            selected = true;
        }

        painter->setRenderHint(QPainter::Antialiasing);
        QRect rect = option.rect;
        QPainterPath bp;
        bp.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);
        painter->setClipPath(bp);
        QPixmap thumbnail;
        // Skill
        // Use cache to make paint faster, cache default limit is 10MB.
        if (!QPixmapCache::find(name, &thumbnail)) {
            // Read thumbnail
            using namespace utils::image;
            const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
            thumbnail = cutSquareImage(DatabaseManager::instance()
                                       ->getImageInfoByName(name).thumbnail, ms);
            QPixmapCache::insert(name, thumbnail);
        }
        painter->drawPixmap(rect, thumbnail);

        // Draw inside border
        QPen p(selected ? BORDER_COLOR_SELECTED : BORDER_COLOR,
               selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
        painter->setPen(p);
        QPainterPathStroker stroker;
        stroker.setWidth(selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
        stroker.setJoinStyle(Qt::RoundJoin);
        QPainterPath borderPath = stroker.createStroke(bp);
        painter->drawPath(borderPath);

        // Draw ticked mark
        if (tickable && selected) {
            QPixmap p = QPixmap(":/images/resources/images/item_selected.png")
                    .scaled(TICKED_MARK_SIZE, TICKED_MARK_SIZE, Qt::KeepAspectRatio,
                            Qt::SmoothTransformation);
            painter->drawPixmap(rect.x() + (rect.width() - TICKED_MARK_SIZE) / 2,
                                rect.y() + (rect.height() - TICKED_MARK_SIZE) / 2,
                                TICKED_MARK_SIZE, TICKED_MARK_SIZE, p);
        }
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.model()->data(index, Qt::SizeHintRole).toSize();
}
