#include "thumbnaildelegate.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "utils/imageutils.h"
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMutex>
#include <QPainter>
#include <QPixmapCache>
#include <QRunnable>
#include <QSvgRenderer>
#include <QtConcurrent>
#include <QThreadPool>

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
    m_indexs.clear();
    m_names.clear();
}

const QModelIndexList ThumbnailDelegate::paintingIndexList()
{
    return m_indexs;
}

const QStringList ThumbnailDelegate::paintingNameList()
{
    return m_names;
}

void ThumbnailDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    if (m_indexs.indexOf(index) == -1) {
        m_indexs << index;
    }

    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        const QString name = datas[0].toString();
//        const QString path = datas[1].toString();
        bool selected = false;
        if (/*(option.state & QStyle::State_MouseOver) &&*/
                (option.state & QStyle::State_Selected) != 0) {
            selected = true;
        }

        if (m_names.indexOf(name) == -1) {
            m_names << name;
        }

        painter->setRenderHints(QPainter::HighQualityAntialiasing |
                                QPainter::SmoothPixmapTransform |
                                QPainter::Antialiasing);
        QRect rect = option.rect;
        QPainterPath bp;
        bp.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);
        painter->setClipPath(bp);
        QPixmap thumbnail;
        // Try to load hight-quality thumbnail from cache
        if (! QPixmapCache::find(name, thumbnail)) {
            // Load thumbnail from model's data
            if (! thumbnail.loadFromData(datas[2].toByteArray())) {
                if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumbnail)) {
                    using namespace utils::image;
                    thumbnail = cutSquareImage(QPixmap(
                    ":/images/resources/images/default_thumbnail.png"),
                    QSize(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE));
                    QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumbnail);
                }
            }
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
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.model()->data(index, Qt::SizeHintRole).toSize();
}

void ThumbnailDelegate::renderThumbnail(const QString &name,
                                        QPixmap &thumbnail) const
{
    const QSize tSize(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
    // Skill
    // Use cache to make paint faster
    if (! QPixmapCache::find(name, &thumbnail)) {
        if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumbnail)) {
            using namespace utils::image;
            thumbnail = cutSquareImage(QPixmap(
                ":/images/resources/images/default_thumbnail.png"), tSize);
            QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumbnail);
        }
    }
}
