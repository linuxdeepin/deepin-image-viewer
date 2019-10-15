/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "thumbnaillistview.h"
#include "thumbnaildelegate.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "controller/viewerthememanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/scrollbar.h"
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
#include <QtMath>

#include "denhancedwidget.h"

namespace {

const int TOP_SPACING = 65;
const int BOTTOM_SPACING = 30;
const int LEFT_MARGIN = 16;
const int RIGHT_MARGIN = 16;
const int ITEM_SPACING = 4;
const int THUMBNAIL_MIN_SIZE = 96;

}  //namespace

ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent),
      m_model(new QStandardItemModel(this))
{
    setViewportMargins(LEFT_MARGIN, 0, RIGHT_MARGIN, 0);
    setSelectionRectVisible(false);
    setIconSize(QSize(THUMBNAIL_MIN_SIZE, THUMBNAIL_MIN_SIZE));
    m_delegate = new ThumbnailDelegate(this);
    connect(m_delegate, &ThumbnailDelegate::thumbnailGenerated,
            this, &ThumbnailListView::updateThumbnail);
    setItemDelegate(m_delegate);
    setModel(m_model);

    m_scrollbar = new ScrollBar(this);
    DEnhancedWidget *enhanced_scrollbar = new DEnhancedWidget(m_scrollbar, m_scrollbar);
    connect(enhanced_scrollbar, &DEnhancedWidget::heightChanged, m_scrollbar,
            [this] {
        m_scrollbar->move(m_scrollbar->x(), utils::common::TOP_TOOLBAR_THEIGHT);
        m_scrollbar->resize(m_scrollbar->width(), m_scrollbar->parentWidget()->height()
                     - utils::common::TOP_TOOLBAR_THEIGHT
                     - utils::common::BOTTOM_TOOLBAR_HEIGHT);
    });
    setVerticalScrollBar(m_scrollbar);
    m_scrollbar->setContextMenuPolicy(Qt::PreventContextMenu);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this, [=]{
        QTimer::singleShot(500, [=]{
            emit enhanced_scrollbar->heightChanged(0);
        });
    });
//    setStyleSheet(utils::base::getFileContent(
//                      ":/resources/common/qss/ThumbnailListView.qss"));

    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setDragEnabled(false);
}

ThumbnailListView::~ThumbnailListView()
{

}

void ThumbnailListView::clearData()
{
    m_model->clear();
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
}

QMutex mutex;
void ThumbnailListView::insertItem(const ItemInfo &info)
{
    QMutexLocker locker(&mutex);
    // Different thread connection cause duplicate insert
    if (indexOf(info.path) != -1)
        return;

    // Lock for model's data reading and setting in different thread
    // Can not use QMutex because the data-operation not in the same class
//    m_delegate->setIsDataLocked(true);
    QStandardItem *item = new QStandardItem;
    item->setData(QVariant(getVariantList(info)), Qt::DisplayRole);
    item->setData(QVariant(iconSize()), Qt::SizeHintRole);
    m_model->appendRow(item);
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

const QStringList ThumbnailListView::selectedPaths() const
{
    QStringList paths;
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        const QVariantList datas =
                index.model()->data(index, Qt::DisplayRole).toList();
        if (datas.length() == 3) {
            paths << datas[1].toString();
        }
    }

    return paths;
}

void ThumbnailListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
//    if (flags & QItemSelectionModel::Clear) {
//        selectionModel()->clear();
//    }

    // For mouse dragging border
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
    QPen sp(utils::common::BORDER_COLOR_SELECTED, 1);
    painter.fillRect(m_selectionRect, QBrush(utils::common::SELECTED_RECT_COLOR));
    painter.setPen(sp);
    painter.drawPath(bp);
}

void ThumbnailListView::wheelEvent(QWheelEvent *e)
{
    QScrollBar *sb = verticalScrollBar();
    if (e->orientation() == Qt::Vertical &&
            sb->value() <= sb->maximum() &&
            sb->value() >= sb->minimum()) {
        if (e->modifiers() == Qt::ControlModifier) {
            emit changeItemSize(e->delta() > 0);
        }
        else {
            QApplication::sendEvent(sb, e);
        }
    }
    else {
        QApplication::sendEvent(horizontalScrollBar(), e);
    }
}

int ThumbnailListView::horizontalOffset() const
{
    const int contentWidth = (iconSize().width() + ITEM_SPACING) * maxColumn();
    return -(this->width() - contentWidth - contentsHMargin()) / 2;
}

int ThumbnailListView::verticalOffset() const
{
    // FIXME:
    // 此处是为了让VIew在最顶端时能有一个topmargin而在滚动 的时候又能穿透到最上面，
    // 但这不是一个好方法
    if (m_scrollbar->value() == 0 ||
            m_scrollbar->value() < (m_scrollbar->maximum() - m_scrollbar->minimum()) / 2)
        return QListView::verticalOffset() - TOP_SPACING;
    else
        return QListView::verticalOffset() + BOTTOM_SPACING;
}

void ThumbnailListView::mouseReleaseEvent(QMouseEvent *e)
{
    m_selectionRect = QRect();
    this->update();
    QListView::mouseReleaseEvent(e);
}

void ThumbnailListView::mousePressEvent(QMouseEvent *e)
{
    m_scrollbar->stopScroll();

    QModelIndex index = indexAt(e->pos());
    if (index.isValid() && e->button() == Qt::RightButton) {
        bool needCallParent = selectedIndexes().length() != 1;

        if (selectedIndexes().length() <= 1) {
            selectionModel()->clear();
            selectionModel()->select(index, QItemSelectionModel::Select);
        }

        if (! needCallParent)
            return;
    }

    QAbstractItemView::mousePressEvent(e);
}

int ThumbnailListView::maxColumn() const
{
    int hMargin = contentsHMargin();

    int c = (parentWidget()->width() - hMargin - ITEM_SPACING*3) / (iconSize().width() + ITEM_SPACING);
    return c;
}

int ThumbnailListView::contentsHMargin() const
{
    return viewportMargins().left() + viewportMargins().right();
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
