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

namespace {

const int BORDER_RADIUS = 0;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;
const QColor BORDER_COLOR = QColor(255, 255, 255, 35);
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");
const QString DEFAULT_THUMB = ":/images/resources/images/default_thumbnail.png";

const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}  // namespace

class TDThumbnailThread : public QThread {
    Q_OBJECT
public:
    explicit TDThumbnailThread(const QString &path);
    const QString path() const;

signals:
    void ready(const QString &path);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString m_path;
};


ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

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
    if (! m_isDataLocked)
        return index.model()->data(index, Qt::SizeHintRole).toSize();
    else
        return QSize(96, 96);
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
    QPixmap thumb;
    using namespace utils::image;
    if (! thumbnailExist(data.path)) {
        if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
            const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
            thumb = cutSquareImage(QPixmap(DEFAULT_THUMB), ms);
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
            // The thumbnail may regenerate by other model
            if (data.thumbnail.toImage().text("Thumb::MTime") != sourceMTime) {
                startThumbnailThread(data.path);
            }
            else {
                thumb = data.thumbnail;
            }
        }
    }

    return thumb;
}

void ThumbnailDelegate::startThumbnailThread(const QString &path) const
{
    for (auto t : m_threads) {
        if (t->path() == path)
            return;
    }
    TDThumbnailThread *t = new TDThumbnailThread(path);
    connect(t, &TDThumbnailThread::finished, this, [=] {
        m_threads.removeAll(t);
        t->deleteLater();
    });
    connect(t, &TDThumbnailThread::ready, this,
            &ThumbnailDelegate::thumbnailGenerated);
    t->start();
}

void ThumbnailDelegate::setIsDataLocked(bool value)
{
    m_isDataLocked = value;
}

#include "thumbnaildelegate.moc"
TDThumbnailThread::TDThumbnailThread(const QString &path)
    : QThread()
    , m_path(path)
{

}

const QString TDThumbnailThread::path() const
{
    return m_path;
}

void TDThumbnailThread::run()
{
    utils::image::generateThumbnail(m_path);
    emit ready(m_path);
}
