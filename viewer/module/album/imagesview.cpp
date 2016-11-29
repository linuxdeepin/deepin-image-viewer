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

#include <math.h>
#include <QShortcut>
#include <DMenu>
#include "daction.h"

using namespace Dtk::Widget;

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const QString MY_FAVORITES_ALBUM = "My favorites";
const QString RECENT_IMPORTED_ALBUM = "Recent imported";

}  // namespace

ImagesView::ImagesView(QWidget *parent)
    : QScrollArea(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBar(new ScrollBar(this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    initShortcut();
    initContent();
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
            showMenuContext(QCursor::pos());
        }
    });
}

void ImagesView::initTopTips()
{
    m_topTips = new TopAlbumTips(this);
}

const QStringList ImagesView::albumPaths()
{
    return dApp->dbM->getPathsByAlbum(m_album);
}

void ImagesView::showMenuContext(QPoint pos) {
    const QStringList paths = selectedPaths();
    const int selectedCount = paths.length();
    bool canSave = true;
    for (QString p : paths) {
        if (! utils::image::imageSupportSave(p)) {
            canSave = false;
            break;
        }
    }
    DMenu subAlbumNameMenu;
    bool albumListIsEmpty = true;
    const QStringList albums = dApp->dbM->getAllAlbumNames();
    DAction* newAlbum = new DAction(tr("Add to new album"), this);
    subAlbumNameMenu.addAction(newAlbum);
    if (!albums.isEmpty())
        subAlbumNameMenu.addSeparator();
    if (! paths.isEmpty()) {
        for (QString album : albums) {
            if (album == MY_FAVORITES_ALBUM) {
                continue;
            }
            else if (! allInAlbum(paths, album)) {
                albumListIsEmpty = false;
                subAlbumNameMenu.addAction(album);
            }
        }
    }

    DMenu mainMenu;
    ////////////////////////////////////////////////////////////////////////////
    if (selectedCount == 1) {
        DAction* viewAct = new DAction(tr("View"), this);
        viewAct->setData(IdView);
        mainMenu.addAction(viewAct);
        DAction* fullSAct = new DAction(tr("Fullscreen"), this);
        fullSAct->setData(IdFullScreen);
        mainMenu.addAction(fullSAct);
    }
    DAction* startSlideSAct = new DAction(tr("Start slide show"), this);
    startSlideSAct->setData(IdStartSlideShow);
    mainMenu.addAction(startSlideSAct);
    DAction* printAct = new DAction(tr("Print"), this);
    printAct->setData(IdPrint);
    mainMenu.addAction(printAct);
    DAction* addToAlbumAct = new DAction(tr("Add to album"), this);
    addToAlbumAct->setData(IdAddToAlbum);
    addToAlbumAct->setMenu(&subAlbumNameMenu);
    if (!albumListIsEmpty)
        mainMenu.addAction(addToAlbumAct);
    mainMenu.addSeparator();
    //////////////////////////////////////////////////////////////////////////
    DAction* cpyAct = new DAction(tr("Copy"), this);
    cpyAct->setData(IdCopy);
    mainMenu.addAction(cpyAct);
    DAction* delAct = new DAction(tr("Throw to Trash"), this);
    delAct->setData(IdThrowToTrash);
    mainMenu.addAction(delAct);
    DAction* removeAct = new DAction(tr("Remove from album"), this);
    removeAct->setData(IdRemoveFromAlbum);
    mainMenu.addAction(removeAct);
    mainMenu.addSeparator();

    /////////////////////////////////////////////////////////////////////////
    DAction* addToFavAct = new DAction(tr("Add to My favorites"), this);
    addToFavAct->setData(IdAddToFavorites);
    if (selectedCount == 1) {
        DAction* removeFavAct = new DAction(tr("Unfavorite"), this);
        removeFavAct->setData(IdRemoveFromFavorites);
        if (!dApp->dbM->isImgExistInAlbum(MY_FAVORITES_ALBUM, paths.first())) {

            mainMenu.addAction(addToFavAct);
        } else {
            mainMenu.addAction(removeFavAct);
        }
    } else {
        bool v = false;
        for (QString img : paths) {
            if (!dApp->dbM->isImgExistInAlbum(MY_FAVORITES_ALBUM, img)) {
                v = true;
                break;
            }
        }
        if (v)
            mainMenu.addAction(addToFavAct);
    }
    ////////////////////////////////////////////////////////////////////////

    if (canSave) {
        mainMenu.addSeparator();
        DAction* rotateAct = new DAction(tr("Rotate clockwise"),
                                         this);
        rotateAct->setData(IdRotateClockwise);
        mainMenu.addAction(rotateAct);
        DAction* antiRotateAct = new DAction(tr("Rotate "
                                                "counterclockwise"), this);
        antiRotateAct->setData(IdRotateCounterclockwise);
        mainMenu.addAction(antiRotateAct);
    }
    mainMenu.addSeparator();

    if (selectedCount == 1) {
        if (canSave) {
            DAction* wallpaperAct = new DAction(tr("Set as wallpaper"), this);
            wallpaperAct->setData(IdSetAsWallpaper);
            mainMenu.addAction(wallpaperAct);
        }
        DAction* displayInFileMAct = new DAction(tr("Display in file manager"),
                                                 this);
        displayInFileMAct->setData(IdDisplayInFileManager);
        mainMenu.addAction(displayInFileMAct);
        DAction* imgInfoAct = new DAction(tr("Image info"), this);
        imgInfoAct->setData(IdImageInfo);
        mainMenu.addAction(imgInfoAct);
    }

    QObject::connect(&mainMenu, &DMenu::triggered, this, [=](QAction* action) {
        if (action->data().toInt() == 0 && action->text() != tr("View")) {
            onMenuItemClicked(IdAddToAlbum, action->text());
        } else {
            onMenuItemClicked(action->data().toInt(), action->text());
        }
    });

    mainMenu.exec(pos);
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
        if (text != tr("Add to new album")) {
            const QString album = text;
            dApp->dbM->insertIntoAlbum(album, paths);
        } else {
            dApp->signalM->createAlbum(paths);
        }
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdThrowToTrash: {
        popupDelDialog(paths);
        break;
    }
    case IdAddToFavorites:
        dApp->dbM->insertIntoAlbum(MY_FAVORITES_ALBUM, paths);
        break;
    case IdRemoveFromFavorites:
        dApp->dbM->removeFromAlbum(MY_FAVORITES_ALBUM, paths);
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

void ImagesView::initShortcut()
{
    // View
    QShortcut *sc = new QShortcut(QKeySequence("Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        QStringList paths = selectedPaths();
        if (paths.isEmpty()) {
            return;
        }

        const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
        const QString path = paths.first();

        emit viewImage(path, viewPaths, false);
    });
    //FullScreen
    sc = new QShortcut(QKeySequence("F11"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        qDebug() << "selectPaths fullScreen";
        QStringList paths = selectedPaths();
        if (paths.isEmpty()) {
            return;
        }

        const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
        const QString path = paths.first();

        emit viewImage(path, viewPaths, true);
    });
    //Start Slide show
    sc = new QShortcut(QKeySequence("F5"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        QStringList paths = selectedPaths();
        if (paths.isEmpty()) {
            return;
        }

        const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
        const QString path = paths.first();

        emit viewImage(path, viewPaths, true);
        emit startSlideShow(viewPaths, path);
    });
    //Print
    sc = new QShortcut(QKeySequence("Ctrl+P"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        QStringList paths = selectedPaths();
        using namespace controller::popup;
        printDialog(paths.first());
    });
    //Copy
    sc = new QShortcut(QKeySequence("Ctrl+C"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        QStringList paths = selectedPaths();
        utils::base::copyImageToClipboard(paths);
    });
    //Throw to trash
    sc = new QShortcut(QKeySequence("Delete"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        popupDelDialog(paths);
    });
    sc = new QShortcut(QKeySequence("Shift+Delete"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        m_view->removeItems(paths);
        dApp->dbM->removeFromAlbum(m_album, paths);
    });
    //Add to My favorites
    sc = new QShortcut(QKeySequence("Ctrl+K"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
       dApp->dbM->insertIntoAlbum(MY_FAVORITES_ALBUM, paths);
    });
    //Remove From My favorites
    sc = new QShortcut(QKeySequence("Shift+Ctrl+K"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        m_view->removeItems(paths);
        dApp->dbM->removeFromAlbum(m_album, paths);
    });
    //Rotate
    sc = new QShortcut(QKeySequence("Ctrl+R"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &ImagesView::rotateImage, path, 90);
            }
        }
    });
    //Counter Rotate
    sc = new QShortcut(QKeySequence("Shift + Ctrl+R"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &ImagesView::rotateImage, path, -90);
            }
        }
    });
    //set as wallpaper
    sc = new QShortcut(QKeySequence("Ctrl+F8"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        if (paths.isEmpty()) {
            return;
        }
        const QString dpath = paths.first();
        dApp->wpSetter->setWallpaper(dpath);
    });
    //Display in file manager
    sc = new QShortcut(QKeySequence("Ctrl+D"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = selectedPaths();
        if (paths.isEmpty()) {
            return;
        }
        const QString dpath = paths.first();
        utils::base::showInFileManager(dpath);;
    });
    // Image info
    sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        const QStringList paths = selectedPaths();
        if (! paths.isEmpty()) {
            dApp->signalM->showImageInfo(paths.first());
        }
    });

    // Select all
    sc = new QShortcut(QKeySequence("Ctrl+A"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        m_view->selectAll();
    });

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
