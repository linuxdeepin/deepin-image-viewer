#include "timelineviewframe.h"
#include "controller/popupmenumanager.h"
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QJsonArray>
#include <QJsonDocument>

const int THUMBNAIL_MAX_SCALE_SIZE = 384;
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

TimelineViewFrame::TimelineViewFrame(const QString &timeline,
                                     bool multiselection,
                                     QWidget *parent)
    : QFrame(parent),
      m_multiselection(multiselection),
      m_iconSize(96, 96),
      m_timeline(timeline),
      m_popupMenu(new PopupMenuManager(this))
{
    QLabel *title = new QLabel(timeline);
    title->setObjectName("TimelineFrameTitle");
    QLabel *separator = new QLabel();
    separator->setObjectName("TimelineSeparator");
    separator->setFixedHeight(1);

    initListView();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(title);
    layout->addWidget(m_listView);
    layout->addWidget(separator);

    connect(m_popupMenu, &PopupMenuManager::menuItemClicked, this, &TimelineViewFrame::onMenuItemClicked);
}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_listView->setFixedWidth(e->size().width());
}

void TimelineViewFrame::initListView()
{
    m_listView = new ThumbnailListView();
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setIconSize(m_iconSize);
    m_listView->setModel( &m_standardModel );
    if (m_multiselection) {
        m_listView->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else {
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    connect(m_listView, &ThumbnailListView::doubleClicked, this, [=] (const QModelIndex & index) {
        emit SignalManager::instance()->viewImage(index.data(Qt::UserRole).toString());
    });
    connect(m_listView, &ThumbnailListView::customContextMenuRequested, [this] {
        m_popupMenu->showMenu(createMenuContent());
    });

    //add data
    QList<DatabaseManager::ImageInfo> list
            = DatabaseManager::instance()->getImageInfosByTime(
                QDateTime::fromString(m_timeline, DATETIME_FORMAT));
    for (DatabaseManager::ImageInfo info : list) {
        insertItem(info);
    }
}

QPixmap TimelineViewFrame::generateSelectedThumanail(const QPixmap &pixmap)
{
    if (m_multiselection) {
        QPixmap target = pixmap;
        QPainter painter(&target);
        QPixmap icon(":/images/icons/resources/images/icons/item_selected.png");
        int selectIconSize = 80;
        painter.drawPixmap((target.width() - selectIconSize) / 2,
                           (target.height() - selectIconSize) / 2,
                           selectIconSize, selectIconSize, icon);

        return target;
    }
    else {
        return pixmap;
    }
}

QPixmap TimelineViewFrame::increaseThumbnail(const QPixmap &pixmap)
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

QString TimelineViewFrame::createMenuContent()
{
    QJsonArray items;
    items.append(createMenuItem(IdView, tr("View")));
    items.append(createMenuItem(IdFullScreen, tr("Fullscreen"), false, "Ctrl+Alt+F"));
    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"), false, "Ctrl+Alt+P"));
    // TODO add album list
    QJsonObject albumObj;
    QJsonArray arry;arry.append(createMenuItem(IdSubMenu, "My favorites"));
    albumObj[""] = QJsonValue(arry);
    items.append(createMenuItem(IdAddToAlbum, tr("Add to album"), false, "", albumObj));

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

QJsonValue TimelineViewFrame::createMenuItem(const MenuItemId id,
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

void TimelineViewFrame::onMenuItemClicked(int menuId)
{
    if (selectedImagesPathList().isEmpty()) {
        return;
    }

    switch (MenuItemId(menuId)) {
    case IdView:
        SignalManager::instance()->viewImage(selectedImagesPathList().first());
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
        SignalManager::instance()->editImage(selectedImagesPathList().first());
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

QSize TimelineViewFrame::iconSize() const
{
    return m_listView->iconSize();
}

void TimelineViewFrame::setIconSize(const QSize &iconSize)
{
    m_listView->setIconSize(iconSize);
}

void TimelineViewFrame::setSelectionModel(QItemSelectionModel *selectionModel)
{
    m_listView->setSelectionModel(selectionModel);
}

void TimelineViewFrame::insertItem(const DatabaseManager::ImageInfo &info)
{
    QStandardItem *item = new QStandardItem();
    item->setData(info.path, Qt::UserRole);
    QIcon icon;
    QPixmap thumbnail = increaseThumbnail(info.thumbnail);
    icon.addPixmap(thumbnail, QIcon::Normal);
    icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
    item->setIcon(icon);
    item->setToolTip(info.name);

    m_standardModel.setItem(m_standardModel.rowCount(), 0, item);
}

void TimelineViewFrame::removeItem(const QString &name)
{
    Q_UNUSED(name)
}

QAbstractItemModel *TimelineViewFrame::model() const
{
    return m_listView->model();
}

QStringList TimelineViewFrame::selectedImagesNameList()
{
    QStringList names;
    for (QModelIndex index : m_listView->selectionModel()->selectedIndexes()) {
        QString path = index.data(Qt::UserRole).toString();
        names << QFileInfo(path).fileName();
    }

    return names;
}

QStringList TimelineViewFrame::selectedImagesPathList()
{
    QStringList paths;
    for (QModelIndex index : m_listView->selectionModel()->selectedIndexes()) {
        paths << index.data(Qt::UserRole).toString();
    }

    return paths;
}

QString TimelineViewFrame::timeline() const
{
    return m_timeline;
}

QSize TimelineViewFrame::viewSize() const
{
    return m_listView->childrenRect().size();
}
