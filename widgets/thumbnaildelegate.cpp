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

const int BORDER_RADIUS = 4;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;
const QColor BORDER_COLOR = QColor(255, 255, 255, 51);
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");

const int TICKED_MARK_SIZE = 24;

const int THUMBNAIL_MAX_SCALE_SIZE = 192;

const int VIEW_UPDATE_INTERVAL = 200;

}

class CacheRunner : public QRunnable
{
public:
    CacheRunner(const QString &name);
    void run() Q_DECL_OVERRIDE;

private:
    QString m_name;
};

class PixmapCacheManager : public QObject
{
    Q_OBJECT
public:
    static PixmapCacheManager *instance();
    void insert(const QString &key, const QPixmap &pixmap);
    bool find(const QString &key, QPixmap *pixmap);
    void remove(const QString &key);

private:
    PixmapCacheManager(QObject *parent = nullptr);

    static PixmapCacheManager *m_cacheManager;
    QMutex m_mutex;
};

#include "thumbnaildelegate.moc"

ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_threadPool = new QThreadPool(this);
    m_threadPool->setMaxThreadCount(1);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, &QTimer::timeout, this, [=] {
        if (QWidget *w = qobject_cast<QWidget *>(this->parent())) {
            w->update();
        }
    });

    QPixmapCache::setCacheLimit(204800);//200MB
}

void ThumbnailDelegate::clearPaintingList()
{
    m_indexs.clear();
    m_names.clear();
}

void ThumbnailDelegate::cancelThumbnailGenerating()
{
    m_threadPool->clear();
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
    m_indexs << index;

    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        const QString name = datas[0].toString();
        const QString path = datas[1].toString();
        const bool tickable = datas[2].toBool();
        bool selected = false;
        if (/*(option.state & QStyle::State_MouseOver) &&*/
                (option.state & QStyle::State_Selected) != 0) {
            selected = true;
        }

        m_names << name;

        painter->setRenderHint(QPainter::Antialiasing);
        QRect rect = option.rect;
        QPainterPath bp;
        bp.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);
        painter->setClipPath(bp);
        QPixmap thumbnail;
        renderThumbnail(path, thumbnail);
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
            painter->drawPixmap(
                        rect.x() + (rect.width() - TICKED_MARK_SIZE) / 2,
                        rect.y() + (rect.height() - TICKED_MARK_SIZE) / 2,
                        TICKED_MARK_SIZE, TICKED_MARK_SIZE,
                        p);
        }
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.model()->data(index, Qt::SizeHintRole).toSize();
}

void ThumbnailDelegate::renderThumbnail(const QString &path,
                                        QPixmap &thumbnail) const
{
    const QString name = QFileInfo(path).fileName();
    const QSize tSize(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
    // Skill
    // Use cache to make paint faster
    if (! PixmapCacheManager::instance()->find(name, &thumbnail)) {
        m_updateTimer->stop();
        CacheRunner *cr = new CacheRunner(name);
        m_threadPool->start(cr);
        // Force View update after thumbnail regenerate
        m_updateTimer->start(VIEW_UPDATE_INTERVAL);

        using namespace utils::image;
        // Try to load low-quality thumbnail during the hight-quality one generating
        if (! PixmapCacheManager::instance()->find(name + "_low", &thumbnail)) {
            // Read low-quality thumbnail failed, read the default icon
            if (! PixmapCacheManager::instance()->find("NO_IMAGE_TMP_KEY",
                                                       &thumbnail)) {
                thumbnail = cutSquareImage(
                    QPixmap(":/images/resources/images/default_thumbnail.png"),
                            tSize);
                PixmapCacheManager::instance()->insert("NO_IMAGE_TMP_KEY",
                                                       thumbnail);
            }
        }
    }
}

CacheRunner::CacheRunner(const QString &name) :
    QRunnable(),
    m_name(name)
{

}

void CacheRunner::run()
{
    auto info = dApp->databaseM->getImageInfoByName(m_name);
    if (! info.thumbnail.isNull()) {
        PixmapCacheManager::instance()->remove(m_name);
        PixmapCacheManager::instance()->remove(m_name + "_low");
        PixmapCacheManager::instance()->insert(m_name, info.thumbnail);
    }
    else {
        // Try to read low-quality thumbnail
        QPixmap lp = utils::image::getThumbnail(info.path, true);
        if (! lp.isNull()) {
            PixmapCacheManager::instance()->insert(m_name + "_low", lp);
        }
    }
}

PixmapCacheManager *PixmapCacheManager::m_cacheManager = NULL;
PixmapCacheManager *PixmapCacheManager::instance()
{
    if (! m_cacheManager) {
        m_cacheManager = new PixmapCacheManager();
    }

    return m_cacheManager;
}

void PixmapCacheManager::insert(const QString &key, const QPixmap &pixmap)
{
    QMutexLocker locker(&m_mutex);

    QPixmapCache::insert(key, pixmap);
}

bool PixmapCacheManager::find(const QString &key, QPixmap *pixmap)
{
    QMutexLocker locker(&m_mutex);

    return QPixmapCache::find(key, pixmap);
}

void PixmapCacheManager::remove(const QString &key)
{
    QMutexLocker locker(&m_mutex);

    QPixmapCache::remove(key);
}

/*!
 * \brief PixmapCacheManager::PixmapCacheManager
 * QPixmapCache::find and QPixmapCache::insert are not thread safe
 * need locker
 * \param parent
 */
PixmapCacheManager::PixmapCacheManager(QObject *parent) : QObject(parent)
{
    // FIXME all static function are not thread save
    // and the locker seem not working
}
