#include "imagesview.h"
#include "application.h"
#include "dscrollbar.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "controller/popupmenumanager.h"
#include "controller/popupdialogmanager.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/importframe.h"
#include "widgets/thumbnaillistview.h"
#include "widgets/scrollbar.h"
#include "frame/deletedialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QStandardItem>
#include <QStackedWidget>
#include <QtConcurrent>
#include <QJsonArray>
#include <QJsonDocument>
#include <math.h>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const QString MY_FAVORITES_ALBUM = "My favorites";
const QString RECENT_IMPORTED_ALBUM = "Recent imported";

}  // namespace

ImagesView::ImagesView(QWidget *parent)
    : QScrollArea(parent),
      m_popupMenu(new PopupMenuManager(this))
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBar(new ScrollBar(this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    initContent();
    updateMenuContents();
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ImagesView::onMenuItemClicked);
}

void ImagesView::setAlbum(const QString &album)
{
    m_album = album;
    m_topTips->setAlbum(m_album);
    m_view->clearData();
    auto infos = dApp->dbM->getInfosByAlbum(album);
    QtConcurrent::run(this, &ImagesView::insertItems, infos);
}

void ImagesView::removeItems(const QStringList &paths)
{
    m_view->removeItems(paths);

    m_topTips->setAlbum(m_album);
    if (m_view->count() == 0) {
        showImportFrame(true);
    }
}

int ImagesView::count() const
{
    return m_view->count();
}

void ImagesView::initListView()
{
    m_view = new ThumbnailListView();
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contentLayout->addWidget(m_view);
    m_contentLayout->addStretch(1);

    connect(this, &ImagesView::rotated,
            m_view, &ThumbnailListView::updateThumbnails);
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            m_view, &ThumbnailListView::updateThumbnails);
    connect(m_view, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        const QString path = m_view->itemInfo(index).path;
        emit viewImage(path, QStringList());
    });
    connect(m_view, &ThumbnailListView::customContextMenuRequested,
            this, [=] (const QPoint &pos) {
        if (m_view->indexAt(pos).isValid()) {
            updateMenuContents();
            m_popupMenu->showMenu();
        }
    });
    connect(m_view, &ThumbnailListView::clicked,
            this, &ImagesView::updateMenuContents);
}

void ImagesView::initTopTips()
{
    m_topTips = new TopAlbumTips(this);
}

const QStringList ImagesView::albumPaths()
{
    return dApp->dbM->getPathsByAlbum(m_album);
}

QString ImagesView::createMenuContent()
{
    const QStringList paths = selectedPaths();
    const int selectedCount = paths.length();
    bool canSave = true;
    for (QString p : paths) {
        if (! utils::image::imageSupportSave(p)) {
            canSave = false;
            break;
        }
    }

    QJsonArray items;
    if (selectedCount == 1) {
        items.append(createMenuItem(IdView, tr("View")));
        items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                    false, "F11"));
    }
    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"), false,
                                "F5"));
    items.append(createMenuItem(IdPrint, tr("Print"), false, "Ctrl+P"));
    const QJsonObject objF = createAlbumMenuObj();
    if (! objF.isEmpty()) {
        items.append(createMenuItem(IdAddToAlbum, tr("Add to album"),
                                    false, "", objF));
    }
    items.append(createMenuItem(IdSeparator, "", true));
    //Hide the export function
    //items.append(createMenuItem(IdExport, tr("Export"), false, ""));
    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdMoveToTrash, tr("Throw to Trash"), false,
                                "Delete"));
    items.append(createMenuItem(IdRemoveFromAlbum, tr("Remove from album"),
                                false, "Shift+Delete"));

    items.append(createMenuItem(IdSeparator, "", true));

    if (selectedCount == 1) {
        if (! dApp->dbM->isImgExistInAlbum(MY_FAVORITES_ALBUM, paths.first()))
            items.append(createMenuItem(IdAddToFavorites,
                tr("Add to My favorites"), false, "Ctrl+K"));
        else
            items.append(createMenuItem(IdRemoveFromFavorites,
                tr("Unfavorite"), false, "Ctrl+Shift+K"));
    } else {
        bool addToFavor = false;
        for (QString path : paths) {
            if (! dApp->dbM->isImgExistInAlbum(MY_FAVORITES_ALBUM, path)) {
                addToFavor = true;
                break;
            }
        }
        if (addToFavor)
            items.append(createMenuItem(IdAddToFavorites,
                        tr("Add to My favorites"), false, "Ctrl+K"));
    }

    if (canSave) {
    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"),
                                false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise,
        tr("Rotate counterclockwise"), false, "Ctrl+Shift+R"));
    }
    items.append(createMenuItem(IdSeparator, "", true));

    if (selectedCount == 1) {
        if (canSave) {
        items.append(createMenuItem(IdSetAsWallpaper, tr("Set as wallpaper"),
                                    false, "Ctrl+F8"));
        }
        items.append(createMenuItem(IdDisplayInFileManager,
            tr("Display in file manager"), false, "Ctrl+D"));
        items.append(createMenuItem(IdImageInfo, tr("Image info"), false,
                                    "Alt+Return"));
    }

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    return QString(QJsonDocument(contentObj).toJson());
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

void ImagesView::insertItem(const DBImgInfo &info, bool update)
{
    using namespace utils::image;
    ThumbnailListView::ItemInfo vi;
    vi.name = info.fileName;
    vi.path = info.filePath;
    vi.thumb = cutSquareImage(getThumbnail(vi.path, true));

    m_view->insertItem(vi);

    if (update) {
        m_view->updateThumbnails();
        m_topTips->setAlbum(m_album);
        showImportFrame(false);
    }
}

void ImagesView::insertItems(const DBImgInfoList &infos)
{
    using namespace utils::image;
    for (auto info : infos) {
        ThumbnailListView::ItemInfo vi;
        vi.name = info.fileName;
        vi.path = info.filePath;
        vi.thumb = cutSquareImage(getThumbnail(vi.path, true));

        m_view->insertItem(vi);
    }
    if (m_view->count() > 0) {
        showImportFrame(false);
    }
}

void ImagesView::updateMenuContents()
{
    m_popupMenu->setMenuContent(createMenuContent());
}

void ImagesView::onMenuItemClicked(int menuId, const QString &text)
{
    QStringList paths = selectedPaths();
    if (paths.isEmpty()) {
        return;
    }

    const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
    const QString path = paths.first();

    switch (MenuItemId(menuId)) {
    case IdView:
        emit viewImage(path, viewPaths);
        break;
    case IdFullScreen:
        emit viewImage(path, viewPaths, true);
        break;
    case IdStartSlideShow:
        emit startSlideShow(viewPaths, path);
        break;
    case IdPrint: {
        using namespace controller::popup;
        printDialog(path);
        break;
    }
    case IdAddToAlbum: {
        const QString album = text.split(SHORTCUT_SPLIT_FLAG).first();
        dApp->dbM->insertIntoAlbum(album, paths);
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdMoveToTrash: {
        popupDelDialog(paths);
        break;
    }
    case IdAddToFavorites:
        dApp->dbM->insertIntoAlbum(MY_FAVORITES_ALBUM, paths);
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        dApp->dbM->removeFromAlbum(MY_FAVORITES_ALBUM, paths);
        updateMenuContents();
        break;
    case IdRemoveFromAlbum:
        m_view->removeItems(paths);
        dApp->dbM->removeFromAlbum(m_album, paths);
        break;
    case IdRotateClockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &ImagesView::rotateImage, path, 90);
            }
        }
        break;
    case IdRotateCounterclockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &ImagesView::rotateImage, path, -90);
            }
        }
        break;
    case IdSetAsWallpaper:
        dApp->wpSetter->setWallpaper(path);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(path);
        break;
    case IdImageInfo:
        emit dApp->signalM->showImageInfo(path);
        break;
    default:
        break;
    }
}

void ImagesView::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    m_rotateList.removeAll(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        emit rotated();
    }
}

void ImagesView::showImportFrame(bool v)
{
    if (v) {
        // For avoid widget destroy
        takeWidget();
        m_contentWidget->setVisible(false);
        setWidget(m_importFrame);
        m_importFrame->setVisible(true);
    }
    else {
        // For avoid widget destroy
        takeWidget();
        m_importFrame->setVisible(false);
        setWidget(m_contentWidget);
        m_contentWidget->setVisible(true);
        updateTopTipsRect();
    }
}

bool ImagesView::allInAlbum(const QStringList &paths, const QString &album)
{
    const QStringList pl = dApp->dbM->getPathsByAlbum(album);
    for (QString path : paths) {
        // One of path is not in album
        if (! pl.contains(path)) {
            return false;
        }
    }
    return true;
}

void ImagesView::updateTopTipsRect()
{
    if (this->widget() != m_contentWidget) {
        m_topTips->setVisible(false);
        return;
    }

    m_topTips->move(0, TOP_TOOLBAR_HEIGHT);
    m_topTips->resize(width(), m_topTips->height());
    const int lm =
            - m_view->hOffset() + m_contentLayout->contentsMargins().left();
    m_topTips->setLeftMargin(lm);
    m_topTips->setVisible(true);
}

QString ImagesView::getCurrentAlbum() const
{
    return m_album;
}

QSize ImagesView::iconSize() const
{
    return m_view->iconSize();
}

void ImagesView::setIconSize(const QSize &iconSize)
{
    m_view->setIconSize(iconSize);
    updateTopTipsRect();
}

QStringList ImagesView::selectedPaths() const
{
    QStringList paths;
    auto infos = m_view->selectedItemInfos();
    for (ThumbnailListView::ItemInfo info : infos) {
        paths << info.path;
    }

    return paths;
}

void ImagesView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    updateTopTipsRect();
}

void ImagesView::showEvent(QShowEvent *e)
{
    // For import from timeline update
    if (count() != dApp->dbM->getImgsCountByAlbum(m_album)) {
        const auto infos = dApp->dbM->getInfosByAlbum(m_album);
        for (auto info : infos) {
            insertItem(info, false);
        }
        m_topTips->setAlbum(m_album);
        if (m_view->count() == 0) {
            showImportFrame(true);
        }
    }

    m_view->updateThumbnails();

    QScrollArea::showEvent(e);
}

void ImagesView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        emit changeItemSize(e->delta() > 0);
        e->accept();
    }
    else {
        QScrollArea::wheelEvent(e);
    }
}

void ImagesView::initContent()
{
    m_contentWidget = new QWidget;
    m_contentWidget->setObjectName("ImagesViewContent");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(10, 70, 6, 20);

    m_importFrame = new ImportFrame(this);
    m_importFrame->setButtonText(tr("Import"));
    m_importFrame->setTitle(tr("Import or drag image to timeline"));
    connect(m_importFrame, &ImportFrame::clicked, this, [=] {
        dApp->importer->showImportDialog(m_album);
    });

    initListView();
    initTopTips();

    setWidget(m_contentWidget);
    m_importFrame->setVisible(false);
}

void ImagesView::updateContent()
{
    if (m_view->count() == 0) {
        // For avoid widget destroy
        takeWidget();
        m_contentWidget->setVisible(false);
        setWidget(m_importFrame);
        m_importFrame->setVisible(true);
    }
    else {
        // For avoid widget destroy
        takeWidget();
        m_importFrame->setVisible(false);
        setWidget(m_contentWidget);
        m_contentWidget->setVisible(true);
        updateTopTipsRect();
    }
}

QJsonObject ImagesView::createAlbumMenuObj()
{
    const QStringList albums = dApp->dbM->getAllAlbumNames();
    const QStringList selectPaths = selectedPaths();

    QJsonArray items;
    if (! selectPaths.isEmpty()) {
        for (QString album : albums) {
            if (album == MY_FAVORITES_ALBUM || album == RECENT_IMPORTED_ALBUM) {
                continue;
            }
            else if (! allInAlbum(selectPaths, album)) {
                items.append(createMenuItem(IdAddToAlbum, album));
            }
        }
    }

    QJsonObject contentObj;
    if (! items.isEmpty()) {
        contentObj[""] = QJsonValue(items);
    }

    return contentObj;
}

void ImagesView::popupDelDialog(const QStringList &paths)
{
    DeleteDialog* delDialog = new DeleteDialog(paths, false, this);
    delDialog->show();
    delDialog->moveToCenter();
    connect(delDialog, &DeleteDialog::buttonClicked, [=](int index){
        delDialog->hide();

        if (index == 1) {
            m_view->removeItems(paths);
            dApp->dbM->removeImgInfos(paths);
            utils::base::trashFiles(paths);
        }
    });
    connect(delDialog, &DeleteDialog::closed,
            delDialog, &DeleteDialog::deleteLater);
}
