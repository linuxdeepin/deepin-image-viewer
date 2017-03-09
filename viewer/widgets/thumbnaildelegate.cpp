#include "thumbnaildelegate.h"
#include "application.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QThread>
#include <QTimer>

class TDThumbnailThread : public QThread {
    Q_OBJECT
public:
    explicit TDThumbnailThread(const ThumbnailDelegate::ItemData &data);
    const QString path() const;

signals:
    void ready(const QString &path);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    ThumbnailDelegate::ItemData m_data;
};


ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThumbnailDelegate::onThemeChanged);
    // Avoid thumbnail-thread being called too ofen
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [=] {
        m_threads.clear();
    });
    t->start(3000);
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
    bp.addRoundedRect(rect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
    painter->setClipPath(bp);

    //**draw transparent background
//    QPixmap pm(12, 12);
//    QPainter pmp(&pm);
//    //TODO: the transparent box
//    //should not be scaled with the image
//    pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
//    pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
//    pmp.end();
    painter->fillRect(rect, QBrush(utils::common::LIGHT_CHECKER_COLOR));
    using namespace utils::image;
    painter->drawPixmap(rect, cutSquareImage(getThumbnail(data.path), rect.size()));

    // Draw inside border
    QPen p(selected ? utils::common::BORDER_COLOR_SELECTED : m_borderColor,
           selected ? utils::common::BORDER_WIDTH_SELECTED : utils::common::BORDER_WIDTH);
    painter->setPen(p);
    QPainterPathStroker stroker;
    stroker.setWidth(selected ? utils::common::BORDER_WIDTH_SELECTED : utils::common::BORDER_WIDTH);
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

QPixmap ThumbnailDelegate::thumbnail(const ThumbnailDelegate::ItemData &data) const
{
    QPixmap thumb = data.thumbnail;
    if (thumb.isNull()) {
        if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
            const QSize ms(utils::common::THUMBNAIL_MAX_SCALE_SIZE,
                           utils::common::THUMBNAIL_MAX_SCALE_SIZE);
            thumb = utils::image::cutSquareImage(QPixmap(m_defaultThumbnail), ms);
            QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumb);
        }
    }
    startThumbnailThread(data);

    return thumb;
}

void ThumbnailDelegate::startThumbnailThread(const ItemData &data) const
{
    if (m_threads.keys().contains(data.path)) {
        return;
    }

    TDThumbnailThread *t = new TDThumbnailThread(data);
    connect(t, &TDThumbnailThread::finished, this, [=] {
        t->deleteLater();
    });
    connect(t, &TDThumbnailThread::ready,
            this, &ThumbnailDelegate::thumbnailGenerated);
    m_threads.insert(data.path, t);
    t->start();
}

void ThumbnailDelegate::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_borderColor = utils::common::DARK_BORDER_COLOR;
        m_defaultThumbnail = utils::common::DARK_DEFAULT_THUMBNAIL;
    } else {
        m_borderColor = utils::common::LIGHT_BORDER_COLOR;
        m_defaultThumbnail = utils::common::DARK_DEFAULT_THUMBNAIL;
    }
}

#include "thumbnaildelegate.moc"
TDThumbnailThread::TDThumbnailThread(const ThumbnailDelegate::ItemData &data)
    : QThread()
    , m_data(data)
{

}

const QString TDThumbnailThread::path() const
{
    return m_data.path;
}

void TDThumbnailThread::run()
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
        emit ready(m_data.path);
    }
    else {
        // The thumbnail may regenerate by other model
        if (m_data.thumbnail.toImage().text("Thumb::MTime") != sourceMTime) {
            generateThumbnail(m_data.path);
            emit ready(m_data.path);
        }
    }
}
