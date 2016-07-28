#include "thumbnaillistview.h"
#include "thumbnaildelegate.h"
#include "controller/databasemanager.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QTimer>

namespace {

const int ITEM_SPACING = 4;
const int THUMBNAIL_MIN_SIZE = 96;
const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}  //namespace

/*!
 * \brief improve the quality of thumbnail in DB
 * \param name
 * \return
 */
QString updateThumbnailQuality(const QString &name)
{
    using namespace utils::image;
    const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
    auto info = DatabaseManager::instance()->getImageInfoByName(name);
    if (info.thumbnail.isNull()) {
        info.thumbnail = cutSquareImage(scaleImage(info.path), ms);
        // Still can't get thumbnail, it must be not supported
        if (info.thumbnail.isNull()) {
            DatabaseManager::instance()->removeImage(name);
            return name;
        }
        DatabaseManager::instance()->updateImageInfo(info);
    }

    return name;
}

ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent),
      m_model(new QStandardItemModel(this)),
      m_multiple(false)
{
    setIconSize(QSize(THUMBNAIL_MIN_SIZE, THUMBNAIL_MIN_SIZE));
    m_delegate = new ThumbnailDelegate(this);
    setItemDelegate(m_delegate);
    setModel(m_model);
    setStyleSheet(utils::base::getFileContent(
                      ":/qss/resources/qss/ThumbnailListView.qss"));

    setMovement(QListView::Free);
    setFrameStyle(QFrame::NoFrame);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);

    viewport()->installEventFilter(this);
    qApp->installEventFilter(this);
    initThumbnailTimer();

    QThreadPool::globalInstance()->setMaxThreadCount(4);
}

ThumbnailListView::~ThumbnailListView()
{
    m_thumbnailWatcher.pause();
    m_thumbnailWatcher.cancel();
    m_thumbnailWatcher.waitForFinished();
}

void ThumbnailListView::setMultiSelection(bool multiple)
{
    m_multiple = multiple;
    if (multiple)
        setSelectionMode(QAbstractItemView::MultiSelection);
    else
        setSelectionMode(QAbstractItemView::SingleSelection);
}

void ThumbnailListView::clearData()
{
    m_model->clear();
}

void ThumbnailListView::updateViewPortSize()
{
    // For expand all items
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, [=] {
        fixedViewPortSize(true);
        t->deleteLater();
    });
    t->start(100);
}

void ThumbnailListView::updateThumbnail(const QString &name)
{
    // Remove cache force view's delegate reread thumbnail
    QPixmapCache::remove(name);

    // Delay for thread generate new thumbnail
    TIMER_SINGLESHOT(200, {viewport()->update();}, this);
}

void ThumbnailListView::setIconSize(const QSize &size)
{
    QListView::setIconSize(size);
    for (int i = 0; i < m_model->rowCount(); i ++) {
        m_model->setData(m_model->index(i, 0), QVariant(size), Qt::SizeHintRole);
    }

    updateViewPortSize();
}

void ThumbnailListView::setTickable(bool v)
{
    for (int i = 0; i < m_model->rowCount(); i ++) {
        QModelIndex index = m_model->index(i, 0);
        if (index.isValid()) {
            ItemInfo info = itemInfo(index);
            info.tickable = v;
            m_model->setData(index, QVariant(getVariantList(info)),
                             Qt::DisplayRole);
        }
    }
}

void ThumbnailListView::insertItem(const ItemInfo &info)
{
    // Diffrent thread connection cause duplicate insert
    if (indexOf(info.name) != -1)
        return;

    QStandardItem *item = new QStandardItem();
    QList<QStandardItem *> items;
    items.append(item);
    m_model->appendRow(items);

    QModelIndex index = m_model->index(m_model->rowCount() - 1, 0);
    m_model->setData(index, QVariant(getVariantList(info)), Qt::DisplayRole);
    m_model->setData(index, QVariant(iconSize()), Qt::SizeHintRole);
    updateViewPortSize();
}

bool ThumbnailListView::removeItem(const QString &name)
{
    const int i = indexOf(name);
    if (i != -1) {
        m_model->removeRow(i);
        updateViewPortSize();
        return true;
    }

    return false;
}

bool ThumbnailListView::contain(const QModelIndex &index) const
{
    return index.model() == m_model;
}

bool ThumbnailListView::isMultiSelection() const
{
    return m_multiple;
}

int ThumbnailListView::indexOf(const QString &name)
{
    for (int i = 0; i < m_model->rowCount(); i ++) {
        const QVariantList datas =
            m_model->data(m_model->index(i, 0), Qt::DisplayRole).toList();
        if (! datas.isEmpty() && datas[0].toString() == name) {
            return i;
        }
    }
    return -1;
}

int ThumbnailListView::count() const
{
    return m_model->rowCount();
}

int ThumbnailListView::hOffset() const
{
    return horizontalOffset();
}

const ThumbnailListView::ItemInfo ThumbnailListView::itemInfo(
        const QModelIndex &index)
{
    const QVariantList datas =
            index.model()->data(index, Qt::DisplayRole).toList();
    ItemInfo info;
    info.name = datas[0].toString();
    info.path = datas[1].toString();
    info.tickable = datas[2].isValid() ? datas[2].toBool() : false;

    return info;
}

const QList<ThumbnailListView::ItemInfo> ThumbnailListView::ItemInfos()
{
    QList<ItemInfo> infos;
    for (int i = 0; i < m_model->rowCount(); i ++) {
        const QVariantList datas =
            m_model->data(m_model->index(i, 0), Qt::DisplayRole).toList();
        ItemInfo info;
        info.name = datas[0].toString();
        info.path = datas[1].toString();
        info.tickable = datas[2].isValid() ? datas[2].toBool() : false;
        infos << info;
    }

    return infos;
}

const QList<ThumbnailListView::ItemInfo> ThumbnailListView::selectedItemInfos()
{
    QList<ItemInfo> infos;
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        const QVariantList datas =
                index.model()->data(index, Qt::DisplayRole).toList();
        ItemInfo info;
        info.name = datas[0].toString();
        info.path = datas[1].toString();
        info.tickable = datas[2].toBool();
        infos << info;
    }

    return infos;
}

bool ThumbnailListView::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == viewport() && event->type() == QEvent::Paint) {
        fixedViewPortSize();
    }
    if (obj == qApp && event->type() == QEvent::ApplicationActivate) {
        // Cache would be auto destroy after some time without no access
        resize(width() + 1, height());
        resize(width() - 1, height());
    }

    return false;
}

void ThumbnailListView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        emit singleClicked(e);
    }
    else {
        if (m_multiple) {
            setSelectionMode(QAbstractItemView::MultiSelection);
        }
        else if(e->modifiers() & Qt::ControlModifier ||
                e->modifiers() & Qt::ShiftModifier){
            setSelectionMode(QAbstractItemView::ExtendedSelection);
        }
        else {
            setSelectionMode(QAbstractItemView::SingleSelection);
            emit singleClicked(e);
        }
    }

    QListView::mousePressEvent(e);
}

void ThumbnailListView::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

void ThumbnailListView::paintEvent(QPaintEvent *e)
{
    m_delegate->clearPaintingList();

    QListView::paintEvent(e);
}

void ThumbnailListView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

int ThumbnailListView::horizontalOffset() const
{
    return -((width()
              - contentsHMargin()
              - maxColumn() * (iconSize().width() + ITEM_SPACING))
             / 2);
}

void ThumbnailListView::onThumbnailResultReady(int index)
{
    Q_UNUSED(index)
    this->update();
}

void ThumbnailListView::fixedViewPortSize(bool proactive)
{
    int hMargin = contentsHMargin();
    int vMargin = contentsVMargin();
    if (! proactive && width() - hMargin == contentsRect().width()
            && height() - vMargin == contentsSize().height())
        return;

    if (contentsSize().isValid()) {
        setFixedHeight(contentsSize().height());
    }
}

int ThumbnailListView::maxColumn() const
{
    int hMargin = contentsHMargin();

    int i = 0;
    while (i * (iconSize().width() + ITEM_SPACING) + hMargin < size().width()) {
        i ++;
    }
    i --;

    return i;
}

int ThumbnailListView::contentsHMargin() const
{
    return contentsMargins().left() + contentsMargins().right();
}

int ThumbnailListView::contentsVMargin() const
{
    return contentsMargins().top() + contentsMargins().bottom();
}

bool caseRow(const QModelIndex &i1, const QModelIndex &i2)
{
    return i1.row() < i2.row();
}

void ThumbnailListView::initThumbnailTimer()
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(2000);
    timer->start();
    connect(timer, &QTimer::timeout, this, [=] {
        if (m_thumbnailCache != m_delegate->paintingNameList()) {
            m_thumbnailCache = m_delegate->paintingNameList();

            m_delegate->cancelThumbnailGenerating();

            // Update DB
            m_thumbnailWatcher.setPaused(true);
            m_thumbnailWatcher.cancel();
            m_thumbnailWatcher.setPaused(false);
            QFuture<QString> qFuture =
                    QtConcurrent::mapped(m_thumbnailCache, updateThumbnailQuality);
            m_thumbnailWatcher.setFuture(qFuture);

            m_paintedIndexs.clear();
            m_paintedIndexs.append(m_delegate->paintingIndexList());
        }
        else if (! m_thumbnailWatcher.isRunning()) {
            // If no request to paint and all the update thread are stopped
            // pre-generate the thumbnail for next frame and the previous frame
            if (m_paintedIndexs.isEmpty())
                return;
            qSort(m_paintedIndexs.begin(), m_paintedIndexs.end(), caseRow);
            int br = m_paintedIndexs.first().row();
            int er = m_paintedIndexs.last().row();
            const int preLoadCount = 8;
            QStringList thumbnailCaches;
            if (br != 0 || er != count() - 1) {
                for (int i = br; i > qMax(0, br - preLoadCount); i--) {
                    m_paintedIndexs << m_model->index(i - 1, 0);
                    thumbnailCaches << itemInfo(m_model->index(i - 1, 0)).name;
                }
                for (int i = er; i < qMin(er + preLoadCount, count() -1); i ++) {
                    m_paintedIndexs << m_model->index(i + 1, 0);
                    thumbnailCaches << itemInfo(m_model->index(i + 1, 0)).name;
                }
            }

            if (thumbnailCaches.isEmpty()) {
                m_thumbnailWatcher.setPaused(true);
                return;
            }
            m_thumbnailWatcher.setPaused(false);
            QFuture<QString> future =
              QtConcurrent::mapped(thumbnailCaches, updateThumbnailQuality);
            m_thumbnailWatcher.setFuture(future);
        }
    });

    m_thumbnailWatcher.setPendingResultsLimit(2);
    connect(&m_thumbnailWatcher, SIGNAL(resultReadyAt(int)),
            this, SLOT(onThumbnailResultReady(int)));
}

const QVariantList ThumbnailListView::getVariantList(const ItemInfo &info)
{
    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.path));
    datas.append(QVariant(info.tickable));

    return datas;
}
