#include "thumbnaillistview.h"
#include "thumbnaildelegate.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QBuffer>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QMutex>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QScrollBar>
#include <QStandardItemModel>

namespace {

const int ITEM_SPACING = 4;
const int THUMBNAIL_MIN_SIZE = 96;

const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");

}  //namespace

ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent),
      m_model(new QStandardItemModel(this))
{
    setSelectionRectVisible(false);
    setIconSize(QSize(THUMBNAIL_MIN_SIZE, THUMBNAIL_MIN_SIZE));
    m_delegate = new ThumbnailDelegate(this);
    connect(m_delegate, &ThumbnailDelegate::thumbnailGenerated,
            this, &ThumbnailListView::updateThumbnail);
    setItemDelegate(m_delegate);
    setModel(m_model);
    setStyleSheet(utils::base::getFileContent(
                      ":/qss/resources/qss/ThumbnailListView.qss"));

    setMovement(QListView::Free);
    setFrameStyle(QFrame::NoFrame);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);

    viewport()->installEventFilter(this);
}

ThumbnailListView::~ThumbnailListView()
{

}

void ThumbnailListView::clearData()
{
    m_model->clear();
}

void ThumbnailListView::updateViewPortSize()
{
    // For expand all items
//    TIMER_SINGLESHOT(100, {fixedViewPortSize(true);}, this)
    QMetaObject::invokeMethod(this, "fixedViewPortSize", Qt::QueuedConnection, Q_ARG(bool, true));
}

void ThumbnailListView::updateThumbnail(const QString &path)
{
    ItemInfo info;
    info.name = QFileInfo(path).fileName();
    info.path = path;
    info.thumb = utils::image::getThumbnail(path, true);
    updateItem(info);
    this->update();
}

void ThumbnailListView::setIconSize(const QSize &size)
{
    QListView::setIconSize(size);
    for (int i = 0; i < m_model->rowCount(); i ++) {
        m_model->setData(m_model->index(i, 0), QVariant(size), Qt::SizeHintRole);
    }

    updateViewPortSize();
}

QMutex mutex;
void ThumbnailListView::insertItem(const ItemInfo &info)
{
    QMutexLocker locker(&mutex);
    // Diffrent thread connection cause duplicate insert
    if (indexOf(info.path) != -1)
        return;

    // Lock for model's data reading and setting in diffrent thread
    // Can not use QMutex because the data-operation not in the same class
    m_delegate->setIsDataLocked(true);
    m_model->appendRow(new QStandardItem());

    QModelIndex index = m_model->index(m_model->rowCount() - 1, 0);
    m_model->setData(index, QVariant(getVariantList(info)), Qt::DisplayRole);
    m_model->setData(index, QVariant(iconSize()), Qt::SizeHintRole);
    m_delegate->setIsDataLocked(false);
    //    updateViewPortSize();
}

void ThumbnailListView::updateItem(const ThumbnailListView::ItemInfo &info)
{
    int i = indexOf(info.path);
    if (i == -1)
        return;
    QModelIndex index = m_model->index(i, 0);
    m_model->setData(index, QVariant(getVariantList(info)), Qt::DisplayRole);
}

void ThumbnailListView::removeItems(const QStringList &paths)
{
    for (QString path : paths) {
        const int i = indexOf(path);
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

int ThumbnailListView::indexOf(const QString &path)
{
    for (int i = 0; i < m_model->rowCount(); i ++) {
        const QVariantList datas =
            m_model->data(m_model->index(i, 0), Qt::DisplayRole).toList();
        if (! datas.isEmpty() && datas[1].toString() == path) {
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

void ThumbnailListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
//    if (flags & QItemSelectionModel::Clear) {
//        selectionModel()->clear();
//    }

    // For mouse draging border
    QRect vr;
    if (flags & QItemSelectionModel::Current) {
        int x = rect.width() < 0 ? rect.x() + rect.width() : rect.x();
        int y = rect.height() < 0 ? rect.y() + rect.height() : rect.y();
        vr = QRect(x, y, qAbs(rect.width()), qAbs(rect.height()));
    }
    else {
        vr = QRect();
    }

    // Do not draw drag border if Qt::ShiftModifier is pressed
    if (flags == QItemSelectionModel::SelectCurrent) {
        m_selectionRect = QRect();
    }
    else {
        m_selectionRect = vr;
    }

//    if (! vr.isEmpty()) {
//        QItemSelection selection;
//        for (auto index : m_paintingIndexs) {
//            if (! visualRect(index).intersected(vr).isEmpty()) {
//                QItemSelection s;
//                s.select(index, index);
//                selection.merge(s, flags);
//            }
//        }
//        selectionModel()->select(selection, flags);
//    }
//    else {
//        QModelIndex index = indexAt(rect.topLeft());
//        selectionModel()->select(index, flags);
//        scrollTo(index);
//    }
//    this->update();
    QListView::setSelection(rect, flags);
    this->update();
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

    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_A) {
        this->selectAll();
    }
    this->update();
}

void ThumbnailListView::paintEvent(QPaintEvent *e)
{
    QListView::paintEvent(e);

    // Draw selection box
    QPainter painter(viewport());
    QPainterPath bp;
    bp.addRect(m_selectionRect);
    QPen sp(BORDER_COLOR_SELECTED, 1);
    painter.setPen(sp);
    painter.drawPath(bp);
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

void ThumbnailListView::mouseReleaseEvent(QMouseEvent *e)
{
    m_selectionRect = QRect();
    this->update();
    QListView::mouseReleaseEvent(e);
}

void ThumbnailListView::mousePressEvent(QMouseEvent *e)
{
    QListView::mousePressEvent(e);
    emit mousePressed(e);
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

const QVariantList ThumbnailListView::getVariantList(const ItemInfo &info)
{
    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.path));
    QByteArray inByteArray;
    QBuffer inBuffer( &inByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    if ( !info.thumb.save( &inBuffer, "JPG", 100 )) { // write inPixmap into inByteArray
//        qDebug() << "Write pixmap to buffer error!" << info.name;
    }
    datas.append(QVariant(inByteArray));

    return datas;
}
