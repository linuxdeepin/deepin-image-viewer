#include "thumbnaillistview.h"
#include "thumbnaildelegate.h"
#include "controller/databasemanager.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QBuffer>
#include <QPaintEvent>
#include <QEvent>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QStandardItemModel>

namespace {

const int ITEM_SPACING = 4;
const int THUMBNAIL_MIN_SIZE = 96;
const int THUMBNAIL_MAX_SCALE_SIZE = 192;

}  //namespace


ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent),
      m_model(new QStandardItemModel(this))
{
    setIconSize(QSize(THUMBNAIL_MIN_SIZE, THUMBNAIL_MIN_SIZE));
    setItemDelegate(new ThumbnailDelegate(this));
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
    const QModelIndex index = m_model->index(indexOf(name), 0);
    if (index.isValid()) {
        m_model->setData(index, QVariant(getVariantList(itemInfo(index))),
                         Qt::DisplayRole);
    }
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

bool ThumbnailListView::contain(const QModelIndex &index) const
{
    return index.model() == m_model;
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
    info.ticked = datas[2].isValid() ? datas[2].toBool() : false;

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
        info.ticked = datas[2].isValid() ? datas[2].toBool() : false;
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
        info.ticked = datas[2].toBool();
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

void ThumbnailListView::mousePressEvent(QMouseEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else if (e->button() == Qt::LeftButton){
        setSelectionMode(QAbstractItemView::SingleSelection);
    }
    emit mousePress(e);
    QListView::mousePressEvent(e);
}

void ThumbnailListView::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
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

const QPixmap ThumbnailListView::getThumbnailByName(const QString &name) const
{
    return DatabaseManager::instance()->getImageInfoByName(name).thumbnail;
}

const QVariantList ThumbnailListView::getVariantList(const ItemInfo &info)
{
    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.path));
    datas.append(QVariant(info.ticked));

    return datas;
}
