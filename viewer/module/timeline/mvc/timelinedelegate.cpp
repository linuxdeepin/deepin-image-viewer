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
#include <QThread>

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

const QString DARK_DEFAULT_THUMBNAIL = ":/resources/dark/images/default_thumbnail.png";
const QString LIGHT_DEFAULT_THUMBNAIL = ":/resources/light/images/default_thumbnail.png";

const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}  // namespace


class TLThumbnailThread : public QThread {
    Q_OBJECT
public:
    explicit TLThumbnailThread(const QString &path);
    const QString path() const;

signals:
    void ready(const QString &path);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString m_path;
};


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
        m_defaultThumbnail = DARK_DEFAULT_THUMBNAIL;
    } else {
        m_borderColor = LIGHT_BORDER_COLOR;
        m_dateColor = LIGHT_DATECOLOR;
        m_seperatorColor = LIGHT_SEPERATOR_COLOR;
        m_defaultThumbnail = LIGHT_DEFAULT_THUMBNAIL;
    }
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
        painter->drawPixmap(rect, thumbnail(data));

        // Draw inside border
        QPen p(selected ? BORDER_COLOR_SELECTED : m_borderColor,
               selected ? BORDER_WIDTH_SELECTED : BORDER_WIDTH);
        painter->setPen(p);
        painter->drawPath(bp);

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

QPixmap TimelineDelegate::thumbnail(const TimelineItem::ItemData &data) const
{
    QPixmap thumb;
    using namespace utils::image;
    const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
    if (! QFileInfo(data.path).exists()) {
        thumb.loadFromData(data.thumbArray);
        return thumb;
    }
    else if (! thumbnailExist(data.path)) {
        if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
            thumb = cutSquareImage(QPixmap(m_defaultThumbnail), ms);
            QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumb);
        }
        startThumbnailThread(data.path);
    }
    else {
        QString thumbMTime = getThumbnail(data.path, true).toImage().text("Thumb::MTime");
        QString sourceMTime = QString::number(QFileInfo(data.path).lastModified().toTime_t());
        // If the file has been changed and thumbnail didn't
        if (thumbMTime != sourceMTime) {
            startThumbnailThread(data.path);
        }
        else {
            thumb.loadFromData(data.thumbArray);
            // The thumbnail may regenerate by other model
            if (thumb.toImage().text("Thumb::MTime") != sourceMTime) {
                thumb = QPixmap();
                startThumbnailThread(data.path);
            }
        }
    }

    return thumb;
}

void TimelineDelegate::startThumbnailThread(const QString &path) const
{
    for (auto t : m_threads) {
        if (t->path() == path)
            return;
    }
    TLThumbnailThread *t = new TLThumbnailThread(path);
    connect(t, &TLThumbnailThread::finished, this, [=] {
        m_threads.removeAll(t);
        t->deleteLater();
    });
    connect(t, &TLThumbnailThread::ready, this,
            &TimelineDelegate::thumbnailGenerated);
    t->start();
}

#include "timelinedelegate.moc"
TLThumbnailThread::TLThumbnailThread(const QString &path)
    : QThread()
    , m_path(path)
{

}

const QString TLThumbnailThread::path() const
{
    return m_path;
}

void TLThumbnailThread::run()
{
    utils::image::generateThumbnail(m_path);
    emit ready(m_path);
}
