#include "thumbnaillistview.h"
#include "thumbnaildelegate.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QBuffer>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QScrollBar>
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
 * return an empty string mean that there is no need to update the thumbnail of model
 */
QString updateThumbnailQuality(const QString &name)
{
    using namespace utils::image;
    const QSize ms(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE);
    auto info = dApp->databaseM->getImageInfoByName(name);
    if (! isImageSupported(info.path))
        return QString();

    if (info.thumbnail.isNull()) {
        info.thumbnail = cutSquareImage(getThumbnail(info.path), ms);
        // Still can't get thumbnail, it must be not supported
        if (info.thumbnail.isNull()) {
            dApp->databaseM->removeImages(QStringList(name));
            return QString();
        }

        dApp->databaseM->updateImageInfo(info);
        return name;
    }
    else {
//        QPixmapCache::insert(name, cutSquareImage(scaleImage(info.path), ms));
        return name;
    }

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

void ThumbnailListView::updateViewPort()
{
    viewport()->update();
}

void ThumbnailListView::updateViewPortSize()
{
    // For expand all items
    TIMER_SINGLESHOT(100, {fixedViewPortSize(true);}, this)
}

void ThumbnailListView::updateThumbnail(const QString &name)
{
    const QModelIndex mi = m_model->index(indexOf(name), 0);
    auto info = itemInfo(mi);
    info.thumb = dApp->databaseM->getImageInfoByName(name).thumbnail;
    m_model->setData(mi, QVariant(getVariantList(info)), Qt::DisplayRole);
}

void ThumbnailListView::updateThumbnails()
{
    m_delegate->clearPaintingList();

    m_thumbnailWatcher.pause();
    m_thumbnailWatcher.cancel();
    m_thumbnailWatcher.waitForFinished();

    // Update painting list
    updateViewPort();

    m_thumbnailTimer->start();
}

void ThumbnailListView::setIconSize(const QSize &size)
{
    QListView::setIconSize(size);
    for (int i = 0; i < m_model->rowCount(); i ++) {
        m_model->setData(m_model->index(i, 0), QVariant(size), Qt::SizeHintRole);
    }

    updateViewPortSize();
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

void ThumbnailListView::removeItems(const QStringList &names)
{
    for (QString name : names) {
        const int i = indexOf(name);
        if (i != -1) {
            m_model->removeRow(i);
        }
    }
    updateViewPortSize();
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
    ItemInfo info;
    if (! index.isValid())
        return info;
    const QVariantList datas =
            index.model()->data(index, Qt::DisplayRole).toList();
    info.name = datas[0].toString();
    info.path = datas[1].toString();
    if (datas[2].isValid()) {
        QPixmap thumb;
        if (thumb.loadFromData(datas[2].toByteArray())) {
            info.thumb = thumb;
        }
    }

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
        if (datas[2].isValid()) {
            QPixmap thumb;
            if (thumb.loadFromData(datas[2].toByteArray())) {
                info.thumb = thumb;
            }
        }
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
        if (datas[2].isValid()) {
            QPixmap thumb;
            if (thumb.loadFromData(datas[2].toByteArray())) {
                info.thumb = thumb;
            }
        }
        infos << info;
    }

    return infos;
}

bool ThumbnailListView::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == viewport() && event->type() == QEvent::Paint) {
        fixedViewPortSize();
    }

    return false;
}

void ThumbnailListView::keyPressEvent(QKeyEvent *e)
{
    QListView::keyPressEvent(e);
    //  QAbstractItemView::selectionChanged is too inefficient
    if (e->key() == Qt::Key_Left ||
            e->key() == Qt::Key_Right ||
            e->key() == Qt::Key_Up ||
            e->key() == Qt::Key_Down) {
        emit clicked(currentIndex());
    }
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
    const QString name = m_thumbnailWatcher.resultAt(index);
    if (! name.isEmpty()) {
        updateThumbnail(name);
    }
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
    m_thumbnailTimer = new QTimer(this);
    m_thumbnailTimer->setInterval(500);
    m_thumbnailTimer->setSingleShot(true);
    connect(m_thumbnailTimer, &QTimer::timeout, this, [=] {
        auto pl = m_delegate->paintingNameList();

        // Update DB
        m_thumbnailWatcher.setPaused(false);
        QFuture<QString> qFuture = QtConcurrent::mapped(pl, updateThumbnailQuality);
        m_thumbnailWatcher.setFuture(qFuture);
    });

    m_thumbnailWatcher.setPendingResultsLimit(2);
    connect(&m_thumbnailWatcher, SIGNAL(resultReadyAt(int)),
            this, SLOT(onThumbnailResultReady(int)));
    connect(&m_thumbnailWatcher, &QFutureWatcher<QString>::finished,
            this, &ThumbnailListView::updateViewPort);

    QPixmapCache::setCacheLimit(204800); // 200MB
}

const QVariantList ThumbnailListView::getVariantList(const ItemInfo &info)
{
    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.path));
    QByteArray inByteArray;
    QBuffer inBuffer( &inByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    if ( !info.thumb.save( &inBuffer, "JPG" )) { // write inPixmap into inByteArray
//        qDebug() << "Write pixmap to buffer error!" << info.name;
    }
    datas.append(QVariant(inByteArray));

    return datas;
}
