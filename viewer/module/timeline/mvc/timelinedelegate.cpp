#include "timelinedelegate.h"
#include "utils/imageutils.h"
#include "application.h"

#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>
#include <QStandardItemModel>

namespace {

const int BORDER_RADIUS = 0;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;

const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 35);
const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 35);

const QColor DARK_DATECOLOR = QColor("#FFFFFF");
const QColor LIGHT_DATECOLOR = QColor(48, 48, 48);

const QColor DARK_SEPERATOR_COLOR = QColor(255, 255, 255, 20);
const QColor LIGHT_SEPERATOR_COLOR = QColor(0, 0, 0, 20);

const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");

const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}

TimelineDelegate::TimelineDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TimelineDelegate::onThemeChanged);
}

void TimelineDelegate::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_borderColor = DARK_BORDER_COLOR;
        m_dateColor = DARK_DATECOLOR;
        m_seperatorColor = DARK_SEPERATOR_COLOR;
    } else {
        m_borderColor = LIGHT_BORDER_COLOR;
        m_dateColor = LIGHT_DATECOLOR;
        m_seperatorColor = LIGHT_SEPERATOR_COLOR;
    }
}
void TimelineDelegate::clearPaintingList()
{
    m_paintingPaths.clear();
}

const QStringList TimelineDelegate::paintingPaths() const
{
    return m_paintingPaths;
}

void TimelineDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    auto data = itemData(index);

    painter->setRenderHints(QPainter::HighQualityAntialiasing |
                            QPainter::SmoothPixmapTransform |
                            QPainter::Antialiasing);

    // Draw Timeline title
    if (data.isTitle) {
        // Draw text
        QPen p(m_dateColor);
        QFont f;f.setPixelSize(12);
        painter->setPen(p);
        painter->setFont(f);
        painter->drawText(option.rect, data.timeline);

        // Draw separator
        QRect r = option.rect;
        r = QRect(r.x(), r.y() + r.height() - 8, r.width(), 1);
        painter->fillRect(r, m_seperatorColor);
    }
    // Draw thumbnail
    else {
        if (data.path.isNull()) return;
        bool selected = false;
        if (/*(option.state & QStyle::State_MouseOver) &&*/
                (option.state & QStyle::State_Selected) != 0) {
            selected = true;
        }

        QRect rect = option.rect;
        QPainterPath bp;
        bp.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);
        painter->setClipPath(bp);
        QPixmap thumb;
        using namespace utils::image;
        // When a thumbnail is changed externally, is no longer used in the data model
        if (! thumbnailExist(data.path)) {
            // Cache thumbnail not exist but model's thumbnail still exist, use the model one
            if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
                const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
                thumb = cutSquareImage(QPixmap(":/images/resources/images/default_thumbnail.png"), ms);
                QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumb);
            }

            // Generate thumbnail and update data from view
            if (m_paintingPaths.indexOf(data.path) == -1) {
                m_paintingPaths << data.path;
            }
        }
        else {
            thumb.loadFromData(data.thumbArray);
        }
        painter->drawPixmap(rect, thumb);

        // Draw inside border
        QPen p(selected ? BORDER_COLOR_SELECTED : m_borderColor,
               selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
        painter->setPen(p);
        QPainterPathStroker stroker;
        stroker.setWidth(selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
        stroker.setJoinStyle(Qt::RoundJoin);
        QPainterPath borderPath = stroker.createStroke(bp);
        painter->drawPath(borderPath);
    }
}

QSize TimelineDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    if (itemData(index).isTitle) {
        return QSize(10, 30);
    }
    else {
        return QSize(30, 50);//index.model()->data(index, Qt::SizeHintRole).toSize();
    }
}

TimelineItem::ItemData TimelineDelegate::itemData(const QModelIndex &index) const
{
    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
    TimelineItem::ItemData data;
    if (datas.count() == 4) { // There is 4 field data inside TimelineData
        data.isTitle = datas[0].toBool();
        data.path = datas[1].toString();
        data.timeline = datas[2].toString();
        data.thumbArray = datas[3].toByteArray();
    }

    return data;
}

