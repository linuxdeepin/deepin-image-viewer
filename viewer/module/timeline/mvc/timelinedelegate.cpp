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

const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");

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
    explicit TLThumbnailThread(const TimelineItem::ItemData &data);
    const QString path() const;

signals:
    void reGenerated(const QString &path);
    void cached(QString path, QPixmap pixmap);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString m_path;
    TimelineItem::ItemData m_data;
};


TimelineDelegate::TimelineDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    // Avoid thumbnail-thread being called too ofen
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [=] {
        m_threads.clear();
    });
    t->start(1000);

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
        f.setWeight(25);
        painter->setPen(p);
        painter->setFont(f);
        QRect r = option.rect;
        QRect texRect = QRect(r.x(), r.y() + 16, r.width(), r.height());
        painter->drawText(texRect, data.timeline);

        // Draw separator

        QRect sepRect = QRect(r.x(), r.y() + r.height() - 8, r.width(), 1);
        painter->fillRect(sepRect, m_seperatorColor);
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
        //**draw transparent background
//        QPixmap pm(12, 12);
//        QPainter pmp(&pm);
//        //TODO: the transparent box
//        //should not be scaled with the image
//        pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
//        pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
//        pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
//        pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
//        pmp.end();
        painter->fillRect(rect, QBrush(LIGHT_CHECKER_COLOR));
        using namespace utils::image;

        painter->drawPixmap(rect, cutSquareImage(getThumbnail(data.path), rect.size()));

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
    thumb.loadFromData(data.thumbArray);
    if (thumb.isNull()) {
        if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
            const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
            thumb = utils::image::cutSquareImage(QPixmap(m_defaultThumbnail), ms);
            QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumb);
        }
    }
    startThumbnailThread(data);

    return thumb;
}

void TimelineDelegate::startThumbnailThread(const TimelineItem::ItemData &data) const
{
    if (m_threads.keys().contains(data.path)) {
        return;
    }

    TLThumbnailThread *t = new TLThumbnailThread(data);
    connect(t, &TLThumbnailThread::finished, this, [=] {
        t->deleteLater();
    });
    connect(t, &TLThumbnailThread::reGenerated,
            this, &TimelineDelegate::thumbnailGenerated);
    m_threads.insert(data.path, t);
    t->start();
}

#include "timelinedelegate.moc"
TLThumbnailThread::TLThumbnailThread(const TimelineItem::ItemData &data)
    : QThread()
    , m_data(data)
{

}

const QString TLThumbnailThread::path() const
{
    return m_data.path;
}

void TLThumbnailThread::run()
{
    using namespace utils::base;
    using namespace utils::image;

    // Do not check the thumbnail for unplug devices' image
    if (onMountDevice(m_data.path) && ! mountDeviceExist(m_data.path)) {
        return;
    }

    QString thumbMTime = getThumbnail(m_data.path, true).toImage().text("Thumb::MTime");
    QString sourceMTime = QString::number(QFileInfo(m_data.path).lastModified().toTime_t());
    // If the file has been changed and thumbnail didn't
    if (thumbMTime != sourceMTime) {
        generateThumbnail(m_data.path);
        // Emit this signal to make model update
        emit reGenerated(m_data.path);
    }
    else {
        QPixmap thumb;
        thumb.loadFromData(m_data.thumbArray);
        // The thumbnail may regenerate by other model
        if (thumb.toImage().text("Thumb::MTime") != sourceMTime) {
            generateThumbnail(m_data.path);
            emit reGenerated(m_data.path);
        }
    }
}
