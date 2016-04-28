#include "imagesview.h"
#include "widgets/thumbnaillistview.h"
#include "controller/databasemanager.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include <QStandardItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int THUMBNAIL_MAX_SCALE_SIZE = 384;
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

}

ImagesView::ImagesView(QWidget *parent)
    : QScrollArea(parent), m_popupMenu(new PopupMenuManager(this))
{
    initContent();

    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ImagesView::onMenuItemClicked);
}

void ImagesView::setAlbum(const QString &album)
{
    m_standardModel.clear();
    QList<DatabaseManager::ImageInfo> infos
            = DatabaseManager::instance()->getImageInfosByAlbum(album);
    for (DatabaseManager::ImageInfo info : infos) {
        QStandardItem *item = new QStandardItem();
        item->setData(info.path, Qt::UserRole);
        QIcon icon;
        QPixmap thumbnail = increaseThumbnail(info.thumbnail);
        icon.addPixmap(thumbnail, QIcon::Normal);
        item->setIcon(icon);
        item->setToolTip(info.name);

        m_standardModel.setItem(m_standardModel.rowCount(), 0, item);
    }

    m_topTips->setAlbum(album);
    m_currentAlbum = album;
}

void ImagesView::initContent()
{
    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("ImagesViewContent");
    m_contentLayout = new QVBoxLayout(m_contentWidget);

    setWidget(m_contentWidget);

    initListView();
    initTopTips();
}

void ImagesView::initListView()
{
    m_listView = new ThumbnailListView(this);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setIconSize(QSize(96, 96));
    m_listView->setModel( &m_standardModel );
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_contentLayout->addWidget(m_listView);

    connect(m_listView, &ThumbnailListView::doubleClicked, this, [=] (const QModelIndex & index) {
        emit SignalManager::instance()->viewImage(index.data(Qt::UserRole).toString(), m_currentAlbum);
    });
    connect(m_listView, &ThumbnailListView::customContextMenuRequested, [this] {
        m_popupMenu->showMenu(createMenuContent());
    });
}

void ImagesView::initTopTips()
{
    m_topTips = new TopAlbumTips(this);
}

QPixmap ImagesView::increaseThumbnail(const QPixmap &pixmap)
{
    QSize targetSize;
    if (pixmap.width() > pixmap.height()) {
        targetSize = QSize(THUMBNAIL_MAX_SCALE_SIZE,
                           (double)THUMBNAIL_MAX_SCALE_SIZE / pixmap.width() *
                           pixmap.height());
    }
    else {
        targetSize = QSize((double)THUMBNAIL_MAX_SCALE_SIZE / pixmap.height() *
                           pixmap.width(), THUMBNAIL_MAX_SCALE_SIZE);
    }

    return pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString ImagesView::createMenuContent()
{
    QJsonArray items;
    items.append(createMenuItem(IdView, tr("View")));
    items.append(createMenuItem(IdFullScreen, tr("Fullscreen"), false, "Ctrl+Alt+F"));
    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"), false, "Ctrl+Alt+P"));
    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdDelete, tr("Delete"), false, "Ctrl+Delete"));
    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdEdit, tr("Edit")));
    items.append(createMenuItem(IdAddToFavorites, tr("Add to favorites"), false, "/"));
    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"), false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise, tr("Rotate counterclockwise"), false, "Ctrl+L"));
    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdLabel, tr("Text tag")));
    items.append(createMenuItem(IdSetAsWallpaper, tr("Set as wallpaper")));
    items.append(createMenuItem(IdDisplayInFileManager, tr("Display in file manager")));
    items.append(createMenuItem(IdImageInfo, tr("Image info"), false, "Ctrl+I"));

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

QJsonValue ImagesView::createMenuItem(const MenuItemId id,
                                      const QString &text,
                                      const bool isSeparator,
                                      const QString &shortcut,
                                      const QJsonObject &subMenu)
{
    return QJsonValue(m_popupMenu->createItemObj(id,
                                                                  text,
                                                                  isSeparator,
                                                                  shortcut,
                                                                  subMenu));
}

void ImagesView::onMenuItemClicked(int menuId)
{
    const QString selectedImage = selectedImagePath();
    if (selectedImage.isEmpty()) {
        return;
    }

    switch (MenuItemId(menuId)) {
    case IdView:
        SignalManager::instance()->viewImage(selectedImage);
        break;
    case IdFullScreen:
        break;
    case IdStartSlideShow:
        break;
    case IdAddToAlbum:
        break;
    case IdCopy:
        break;
    case IdDelete:
        break;
    case IdEdit:
        SignalManager::instance()->editImage(selectedImage);
        break;
    case IdAddToFavorites:
        break;
    case IdRotateClockwise:
        break;
    case IdRotateCounterclockwise:
        break;
    case IdLabel:
        break;
    case IdSetAsWallpaper:
        break;
    case IdDisplayInFileManager:
        break;
    case IdImageInfo:
        break;
    default:
        break;
    }
}

void ImagesView::updateContentRect()
{
    int minWidth = getMinContentsWidth();
    int hMargin = (width() - minWidth) / 2;
    m_contentLayout->setContentsMargins(hMargin, 60, hMargin, 10);
    m_contentWidget->setFixedWidth(width());
    m_listView->setFixedWidth(width());
}

void ImagesView::updateTopTipsRect()
{
    m_topTips->move(0, TOP_TOOLBAR_HEIGHT);
    m_topTips->resize(width(), m_topTips->height());
}

int ImagesView::getMinContentsWidth()
{
    int itemSpacing = 10;
    int viewHMargin = 14 * 2;
    int holdCount = floor((double)(width() - itemSpacing - viewHMargin)
                          / (iconSize().width() + itemSpacing));
    return (iconSize().width() + itemSpacing) * holdCount + itemSpacing + viewHMargin;
}

QSize ImagesView::iconSize() const
{
    return m_listView->iconSize();
}

void ImagesView::setIconSize(const QSize &iconSize)
{
    m_listView->setIconSize(iconSize);
}

QString ImagesView::selectedImagePath() const
{
    QModelIndexList indexList = m_listView->selectionModel()->selectedIndexes();
    if (indexList.isEmpty()) {
        return QString();
    }
    else {
        return indexList.first().data(Qt::UserRole).toString();
    }
}

void ImagesView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    updateContentRect();
    updateTopTipsRect();
}
