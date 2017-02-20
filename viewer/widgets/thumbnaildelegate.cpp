#include "thumbnaildelegate.h"
#include "application.h"
#include "utils/imageutils.h"
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QThread>
#include <QTimer>

namespace {

const int BORDER_RADIUS = 0;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;
const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 26);
const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 26);
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");
const QString DARK_DEFAULT_THUMB = ":/resources/dark/images/"
                                   "default_thumbnail.png";
const QString LIGHT_DEFAULT_THUMB = ":/resources/light/images/"
                                   "default_thumbnail.png";
const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}  // namespace

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
    bp.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);
    painter->setClipPath(bp);
    painter->drawPixmap(rect, thumbnail(data));

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
            const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
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
        m_borderColor = DARK_BORDER_COLOR;
        m_defaultThumbnail = DARK_DEFAULT_THUMB;
    } else {
        m_borderColor = LIGHT_BORDER_COLOR;
        m_defaultThumbnail = LIGHT_DEFAULT_THUMB;
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
