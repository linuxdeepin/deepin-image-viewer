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
#include "albumsview.h"
#include "albumdelegate.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/signalmanager.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "controller/viewerthememanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/dialogs/albumdeletedialog.h"
#include "widgets/scrollbar.h"
#include "denhancedwidget.h"

#include <QBuffer>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QShortcut>
#include <QStyleFactory>
#include <QJsonDocument>

namespace {

const QString VIEW_GROUP = "SHORTCUTVIEW";
const QString ALBUM_GROUP = "SHORTCUTALBUM";
const QString MY_FAVORITES_ALBUM = "My favorite";
const QString RECENT_IMPORTED_ALBUM = "Recent imported";
const int ITEM_SPACING = 61;
const QSize ITEM_DEFAULT_SIZE = QSize(152, 168);

QString ss(const QString &text, const QString &group)
{
    QString str = dApp->setter->value(group, text).toString();
    str.replace(" ", "");
    return str;
}

}  // namespace

AlbumsView::AlbumsView(QWidget *parent)
    : QListView(parent),
      m_itemSize(ITEM_DEFAULT_SIZE)
{
    setMouseTracking(true);
    setObjectName("AlbumsView");

    m_delegate = new AlbumDelegate(this);
    // Key event and focusout event will active this signal twice
    // Use QueuedConnection to avoid crash
    connect(m_delegate, &AlbumDelegate::editingFinished,
            this, &AlbumsView::destroyEditor, Qt::QueuedConnection);

    setItemDelegate(m_delegate);
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setDragEnabled(false);

    QScrollBar* sb = new QScrollBar(this);
    DEnhancedWidget *enhanced_scrollbar = new DEnhancedWidget(sb, sb);
    connect(enhanced_scrollbar, &DEnhancedWidget::heightChanged, this, [sb] {
        sb->move(sb->x(), utils::common::TOP_TOOLBAR_THEIGHT);
        sb->resize(sb->width(), sb->parentWidget()->height()
                     - utils::common::TOP_TOOLBAR_THEIGHT
                     - utils::common::BOTTOM_TOOLBAR_HEIGHT);
    });
    setVerticalScrollBar(sb);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this, [=]{
        QTimer::singleShot(500, [=]{
            emit enhanced_scrollbar->heightChanged(0);
        });
    });

    initShortcut();
    sb->setContextMenuPolicy(Qt::PreventContextMenu);
    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    connect(m_menu, &QMenu::triggered, this, &AlbumsView::onMenuItemClicked);

    connect(this, &AlbumsView::doubleClicked,
            this, &AlbumsView::onDoubleClicked);
    connect(this, &AlbumsView::clicked, this, &AlbumsView::onClicked);
    connect(this, &AlbumsView::customContextMenuRequested,
            this, [=] (const QPoint &pos) {
        QModelIndex index = indexAt(pos);
        if (! isCreateIcon(index)) {
            updateMenuContent(index);
            m_menu->popup(QCursor::pos());
        }
    });
    connect(selectionModel(), &QItemSelectionModel::currentChanged,
            this, [=] (const QModelIndex &current) {
        updateMenuContent(current);
    });
    connect(dApp->setter, &ConfigSetter::valueChanged,
            this, [=] (const QString &group) {
        if (group == VIEW_GROUP || group == ALBUM_GROUP) {
            updateMenuContent(currentIndex());
        }
    });
    connect(Importer::instance(), &Importer::imported, this, [=] (bool success) {
        if (success) {
            updateView();
        }
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &AlbumsView::updateView);
}

QModelIndex AlbumsView::addAlbum(const DBAlbumInfo &info)
{
    // AlbumName ImageCount BeginTime EndTime
    QStringList paths = DBManager::instance()->getPathsByAlbum(info.name);

    QByteArray thumbnailByteArray;
    QBuffer inBuffer( &thumbnailByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    // write inPixmap into inByteArray
    QString priPath;
    if (!paths.isEmpty())
        priPath = paths.first();
    else
        priPath = " ";

    if (! priPath.isEmpty()){
        QPixmap p = utils::image::getThumbnail(priPath);
        if (! p.save(&inBuffer, "JPG", 100)) {
            qWarning() << "Can't get thumbnail for album: " << info.name;
        }
    }

    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.count));
    datas.append(QVariant(info.beginTime));
    datas.append(QVariant(info.endTime));
    datas.append(QVariant(thumbnailByteArray));

    removeCreateIcon();

    QModelIndex index;
    const int ti = indexOf(info.name);
    // Already exist, update the data
    if (ti != -1) {
        index = m_model->index(ti, 0);
    }
    // Not exist, create new item
    else {
        QStandardItem *item = new QStandardItem();
        QList<QStandardItem *> items;
        items.append(item);
        m_model->appendRow(items);

        index = m_model->index(m_model->rowCount() - 1, 0);
    }

    m_model->setData(index, QVariant(datas), Qt::DisplayRole);
    m_model->setData(index, QVariant(m_itemSize), Qt::SizeHintRole);
    appendCreateIcon();
    return index;
}

QSize AlbumsView::itemSize() const
{
    return m_itemSize;
}

void AlbumsView::setItemSizeMultiple(int multiple)
{
    const int w = ITEM_DEFAULT_SIZE.width() + multiple * 32;
    const int h = (1.0 *ITEM_DEFAULT_SIZE.height() / ITEM_DEFAULT_SIZE.width()) * w;
    m_itemSize = QSize(w, h);
    updateView();

    // SKILL
    // This signal must be emitted when the sizeHint() of index changed.
    // View has been set Uniform Item Sizes as true, so all of the items's size
    // is change with the last item on the view
    emit m_delegate->sizeHintChanged(m_model->index(m_model->rowCount() - 1, 0));
}

void AlbumsView::mousePressEvent(QMouseEvent *e)
{
    if (! indexAt(e->pos()).isValid()) {
        this->selectionModel()->clearSelection();
    }

    destroyEditor(currentIndex());

    QListView::mousePressEvent(e);
}

void AlbumsView::showEvent(QShowEvent *e)
{
    QListView::showEvent(e);
    // Aways has Favorites album
    if (!DBManager::instance()->isAlbumExistInDB(MY_FAVORITES_ALBUM))
        DBManager::instance()->insertIntoAlbum(MY_FAVORITES_ALBUM, QStringList(" "));
    updateView();
}

void AlbumsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        emit changeItemSize(e->delta() > 0);
        e->accept();
    }
    else {
        QListView::wheelEvent(e);
    }
}

void AlbumsView::initShortcut()
{
    QShortcut *sc = new QShortcut(QKeySequence(ss("New album", ALBUM_GROUP)), this);
    connect(sc, &QShortcut::activated, this, &AlbumsView::createAlbum);

    connect(dApp->setter, &ConfigSetter::valueChanged,
            this, [=] (const QString &group) {
        if (group == ALBUM_GROUP) {
            sc->setKey(QKeySequence(ss("New album", ALBUM_GROUP)));
        }
    });
}

int AlbumsView::horizontalOffset() const
{
    double spacing = 1.0 * (width() % (m_itemSize.width() + ITEM_SPACING) - ITEM_SPACING) / 2;
    // 0 is critical point, DO NOT use 0
    spacing = spacing == 0 ? -1 : spacing;
    // Not enough for item spacing
    if (spacing < 0) {
        spacing = 1.0 * (m_itemSize.width() + ITEM_SPACING) / 2 + spacing;
    }
    return -spacing;
}

bool AlbumsView::isCreateIcon(const QModelIndex &index) const
{
    return m_model->data(index, Qt::DisplayRole).toList().isEmpty();
}

void AlbumsView::appendAction(int id, const QString &text, const QString &shortcut, bool enable)
{
    QAction *ac = new QAction(m_menu);
    addAction(ac);
    ac->setDisabled(! enable);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    ac->setShortcut(QKeySequence(shortcut));
    m_menu->addAction(ac);
}

const QStringList AlbumsView::paths(const QString &album) const
{
    return DBManager::instance()->getPathsByAlbum(album);
}

QString AlbumsView::getAlbumName(const QModelIndex &index)
{
    QString albumName = "";
    //TODO: if index isn't valid app will crashed,
    //index need be valid to access.
    if (!index.isValid())
        return "";

    QList<QVariant> datas =
            index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        albumName = datas[0].toString();
    }

    return albumName;
}

const QString AlbumsView::getNewAlbumName()
{
    const QString nan = tr("Unnamed");
       int num = 1;
       QString albumName = nan;
       while(DBManager::instance()->isAlbumExistInDB(albumName)) {
           num++;
           albumName = nan + QString::number(num);
       }
       return (const QString)(albumName);
}

void AlbumsView::updateMenuContent(const QModelIndex &index)
{
    if (! index.isValid() || isCreateIcon(index))
        return;

    m_menu->clear();
    qDeleteAll(this->actions());

    bool isSpecial = false;
    bool isEmpty = false;
    QList<QVariant> datas =
            index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        const QString album = datas[0].toString();
        isSpecial = album == MY_FAVORITES_ALBUM;
        isEmpty = DBManager::instance()->getImgsCountByAlbum(album) < 1;
    }
    appendAction(IdView, tr("View"), ss("View", VIEW_GROUP));
    appendAction(IdStartSlideShow,
                 tr("Slide show"), ss("Slide show", VIEW_GROUP), ! isEmpty);
    m_menu->addSeparator();
    if (! isSpecial)
        appendAction(IdRename, tr("Rename"), ss("Rename", ALBUM_GROUP));
    appendAction(IdCopy, tr("Copy"), ss("Copy", VIEW_GROUP), ! isEmpty);
    if (! isSpecial)
        appendAction(IdDelete, tr("Delete"), ss("Delete", ALBUM_GROUP));
}

void AlbumsView::onClicked(const QModelIndex &index)
{
    if (isCreateIcon(index)) {
        createAlbum();
    }
}

void AlbumsView::onMenuItemClicked(QAction *action)
{
    if (isCreateIcon(currentIndex()))
        return;
    const QString albumName = getAlbumName(currentIndex());
    const int id = action->property("MenuID").toInt();
    switch (MenuItemId(id)) {
    case IdCreate:
        createAlbum();
        break;
    case IdView:
        if (m_delegate->isEditFinished()) {
            emit openAlbum(albumName);
        }
        else {
            destroyEditor(currentIndex());
        }
        break;
    case IdStartSlideShow:
        emit startSlideShow(paths(albumName));
        break;
    case IdRename:
        if (m_delegate->isEditFinished()) {
            QModelIndex renameIndex = this->currentIndex();
            this->clearSelection();
            openPersistentEditor(renameIndex);
        }
        break;
    case IdExport:
         Exporter::instance()->exportAlbum(albumName);
        break;
    case IdCopy: {
        QStringList paths = DBManager::instance()->getPathsByAlbum(albumName);
        paths.removeAll(" ");
        utils::base::copyImageToClipboard(paths);
        break;
    }
    case IdDelete: {
        if (!albumName.isEmpty())
            popupDelDialog(albumName);
        break;
    }
    default:
        break;
    }
}

void AlbumsView::onDoubleClicked(const QModelIndex &index)
{
    if (! isCreateIcon(index)) {
        emit openAlbum(getAlbumName(index));
    }
}

void AlbumsView::removeCreateIcon()
{
    // The create icon must in the end of rows
    const QModelIndex li = m_model->index(m_model->rowCount() - 1, 0);
    if (m_model->rowCount() > 0 &&
            isCreateIcon(li)) {
        m_model->removeRow(m_model->rowCount() - 1);
    }
}

void AlbumsView::appendCreateIcon()
{
    const QModelIndex li = m_model->index(m_model->rowCount() - 1, 0);
    if (! isCreateIcon(li)) {
        QStandardItem *item = new QStandardItem();
        QList<QStandardItem *> items;
        items.append(item);
        m_model->appendRow(items);

        QModelIndex index = m_model->index(m_model->rowCount() - 1, 0);
        m_model->setData(index, QVariant(m_itemSize), Qt::SizeHintRole);
    }
}

void AlbumsView::destroyEditor(const QModelIndex &index)
{
    if (! index.isValid())
        return;
    if (indexWidget(index)) {
        closePersistentEditor(index);
    }
}

void AlbumsView::createAlbum()
{
    destroyEditor(currentIndex());
    const QString name = getNewAlbumName();
     DBManager::instance()->insertIntoAlbum(name, QStringList(" "));
    QModelIndex index = addAlbum(DBManager::instance()->getAlbumInfo(name));
    setCurrentIndex(index);
    // Make sure the created icon is visible
    scrollTo(m_model->index(index.row() + 1, 0, index.parent()));
    openPersistentEditor(index);
    emit albumCreated();
}

void AlbumsView::updateView()
{
    if (! isVisible())
        return;

    // Make those special album always show at front
    addAlbum(DBManager::instance()->getAlbumInfo(MY_FAVORITES_ALBUM));
    QStringList albums = DBManager::instance()->getAllAlbumNames();
    albums.removeAll(MY_FAVORITES_ALBUM);
    for (const QString album : albums) {
        addAlbum(DBManager::instance()->getAlbumInfo(album));
    }
}

int AlbumsView::indexOf(const QString &name) const
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

void AlbumsView::popupDelDialog(const QString &albumName) {
    QStringList paths = DBManager::instance()->getPathsByAlbum(albumName);
    AlbumDeleteDialog *add = new AlbumDeleteDialog(paths);
    add->showInCenter(window());
    connect(add, &AlbumDeleteDialog::buttonClicked, this, [=] (int index) {
        if (index == 1) {
            if (albumName != MY_FAVORITES_ALBUM
                    && albumName != RECENT_IMPORTED_ALBUM
                    && ! isCreateIcon(currentIndex())) {
                DBManager::instance()->removeAlbum(albumName);
                m_model->removeRow(currentIndex().row());
                emit albumRemoved();
            }
        }
    });
}
