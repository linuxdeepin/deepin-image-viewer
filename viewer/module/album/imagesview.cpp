#include "imagesview.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "controller/popupdialogmanager.h"
#include "controller/signalmanager.h"

#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/importframe.h"
#include "widgets/scrollbar.h"
#include "widgets/dialogs/filedeletedialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QMenu>
#include <QStandardItem>
#include <QStackedWidget>
#include <QStyleFactory>
#include <QtConcurrent>
#include <QJsonArray>
#include <QJsonDocument>
#include <math.h>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString RECENT_IMPORTED_ALBUM = "Recent imported";
const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";

QString ss(const QString &text)
{
    return dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString();
}

}  // namespace

class LoadThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadThread(const QString &album);

    void setDone(bool done);

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void ready(ThumbnailListView::ItemInfo);

private:
    bool m_done;
    QString m_album;
};

#include "imagesview.moc"

ImagesView::ImagesView(QWidget *parent)
    : QScrollArea(parent)
{
    setObjectName("ImagesView");
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBar(new ScrollBar(this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    initContent();
    initPopupMenu();

    connect(dApp->signalM, &SignalManager::insertedIntoAlbum,
            this, &ImagesView::initItems);
    connect(dApp->setter, &ConfigSetter::valueChanged, this, [=] (const QString &g) {
       if (g == SHORTCUTVIEW_GROUP) {
           updateMenuContents();
       }
    });
}

void ImagesView::cancelLoadImages()
{
    for (auto t : m_loadingThreads) {
        t->setDone(true);
        t->disconnect();
        t->quit();
        t->wait();
        t->deleteLater();
    }
    m_loadingThreads.clear();
}

void ImagesView::setAlbum(const QString &album)
{
    m_album = album;
    m_topTips->setAlbum(m_album);
    m_view->clearData();

    initItems();
}

void ImagesView::removeItems(const QStringList &paths)
{
    m_view->removeItems(paths);

    m_topTips->setAlbum(m_album);
    updateContent();
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

    connect(m_view, &ThumbnailListView::clicked,
            this, &ImagesView::updateMenuContents);
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &ImagesView::updateMenuContents);
    connect(m_view, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        const QString path = m_view->itemInfo(index).path;
        emit viewImage(path, QStringList());
    });
    connect(m_view, &ThumbnailListView::customContextMenuRequested,
            this, [=] (const QPoint &pos) {
        if (m_view->indexAt(pos).isValid()) {
            updateMenuContents();
            m_menu->popup(QCursor::pos());
        }
    });
    connect(m_view, &ThumbnailListView::mousePressed, this, [=] {
        ScrollBar *sb = dynamic_cast<ScrollBar *>(verticalScrollBar());
        if (sb) {
            sb->stopScroll();
        }
    });
}

void ImagesView::initTopTips()
{
    m_topTips = new TopAlbumTips(this);
}

void ImagesView::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(m_menu);
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    const QString ss = dApp->setter->value(SHORTCUTVIEW_GROUP, text, shortcut).toString();
    ac->setShortcut(QKeySequence(ss));
    m_menu->addAction(ac);
}

const QStringList ImagesView::albumPaths()
{
    return dApp->dbM->getPathsByAlbum(m_album);
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
        m_view->update();
        m_topTips->setAlbum(m_album);
        updateContent();
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
    updateContent();
}

void ImagesView::updateMenuContents()
{
    const QStringList paths = selectedPaths();

    if (paths.isEmpty())
        return;

    m_menu->clear();
    qDeleteAll(this->actions());

    const int selectedCount = paths.length();
    auto supportPath = std::find_if_not(paths.cbegin(), paths.cend(),
                                        utils::image::imageSupportSave);
    bool canSave = supportPath == paths.cend();

    if (selectedCount == 1) {
        appendAction(IdView, tr("View"), ss("View"));
        appendAction(IdFullScreen, tr("Fullscreen"), ss("Fullscreen"));
    }
    appendAction(IdStartSlideShow, tr("Start slideshow"), ss("Start slideshow"));
    appendAction(IdPrint, tr("Print"), ss("Print"));
    QMenu *am = createAlbumMenu();
    if (am) {
        m_menu->addMenu(am);
    }
    m_menu->addSeparator();
    /**************************************************************************/
    appendAction(IdCopy, tr("Copy Image Path"), ss("Copy Image Path"));
    if (selectedCount == 1)
        appendAction(IdCopyToClipboard, tr("Copy Image"), ss("Copy Image"));
    appendAction(IdMoveToTrash, tr("Throw to trash"), ss("Throw to trash"));
    appendAction(IdRemoveFromAlbum, tr("Remove from album"), ss("Remove from album"));
    m_menu->addSeparator();
    /**************************************************************************/
    bool isCollected = false, unFavor = false;
    for (QString img : paths) {
        if (dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, img)) {
            isCollected = true;
            continue;
        } else {
            unFavor = true;
        }
    }
    if (!unFavor && paths.length() == 1) {
        appendAction(IdRemoveFromFavorites, tr("Unfavorite"), ss("Unfavorite"));
    }
    else if (!isCollected || unFavor ) {
        appendAction(IdAddToFavorites,
                     tr("Add to my favorite"), ss("Add to my favorite"));
    }
    m_menu->addSeparator();
    /**************************************************************************/
    if (canSave) {
        m_menu->addSeparator();
        appendAction(IdRotateClockwise,
                     tr("Rotate clockwise"), ss("Rotate clockwise"));
        appendAction(IdRotateCounterclockwise,
                     tr("Rotate counterclockwise"), ss("Rotate counterclockwise"));
    }
    /**************************************************************************/
    if (selectedCount == 1)  {
        if (canSave) {
            appendAction(IdSetAsWallpaper,
                         tr("Set as wallpaper"), ss("Set as wallpaper"));
        }
        appendAction(IdDisplayInFileManager,
                     tr("Display in file manager"), ss("Display in file manager"));
    }
    appendAction(IdImageInfo, tr("Image info"), ss("Image info"));
}

void ImagesView::onMenuItemClicked(QAction *action)
{
    QStringList paths = selectedPaths();
    paths.removeAll(QString(""));
    if (paths.isEmpty()) {
        return;
    }

    const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
    const QString path = paths.first();

    const int id = action->property("MenuID").toInt();
    switch (MenuItemId(id)) {
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
        const QString album = action->data().toString();
        if (album != "Add to new album") {
            dApp->dbM->insertIntoAlbum(album, paths);
        }
        else {
            emit dApp->signalM->createAlbum(paths);
        }
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdCopyToClipboard:
        utils::base::copyOneImageToClipboard(path);
        break;
    case IdMoveToTrash: {
        popupDelDialog(paths);
        break;
    }
    case IdAddToFavorites:
        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRemoveFromFavorites:
        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
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

    updateMenuContents();
}

void ImagesView::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    m_rotateList.removeAll(path);
    m_view->updateThumbnail(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        emit rotateFinished();
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

void ImagesView::initItems()
{
    LoadThread *t = new LoadThread(m_album);
    qRegisterMetaType<QVector<int>>("QVector<int>");
    connect(t, &LoadThread::ready, m_view, &ThumbnailListView::insertItem,
            Qt::DirectConnection);
    connect(t, &LoadThread::finished, this, [=] {
        disconnect(t, &LoadThread::ready, m_view, &ThumbnailListView::insertItem);
        m_loadingThreads.removeAll(t);
        t->deleteLater();
    });
    m_loadingThreads.append(t);
    t->start();
    updateContent();
}

void ImagesView::initPopupMenu()
{
    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    connect(m_menu, &QMenu::triggered, this, &ImagesView::onMenuItemClicked);
    connect(this, &ImagesView::rotateFinished,
            this, &ImagesView::updateMenuContents);
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
    if (dApp->dbM->getImgsCountByAlbum(m_album) < 1) {
        // For avoid widget destroy
        takeWidget();
        m_contentWidget->setVisible(false);
        setWidget(m_importFrame);
        m_importFrame->setVisible(true);
    }
    else if (widget() != m_contentWidget){
        // For avoid widget destroy
        takeWidget();
        m_importFrame->setVisible(false);
        setWidget(m_contentWidget);
        m_contentWidget->setVisible(true);
        updateTopTipsRect();
    }
}

QMenu *ImagesView::createAlbumMenu()
{
    QMenu *am = new QMenu(tr("Add to album"));
    am->setStyle(QStyleFactory::create("dlight"));
    QStringList albums = dApp->dbM->getAllAlbumNames();
    albums.removeAll(FAVORITES_ALBUM_NAME);
    const QStringList sps = selectedPaths();

    QAction *ac = new QAction(am);
    ac->setProperty("MenuID", IdAddToAlbum);
    ac->setText(tr("Add to new album"));
    ac->setData(QString("Add to new album"));
    am->addAction(ac);
    am->addSeparator();
    for (QString album : albums) {
        const QStringList paths = dApp->dbM->getPathsByAlbum(album);
        for (QString sp : sps) {
            if (! paths.contains(sp)) {
                QAction *ac = new QAction(am);
                ac->setProperty("MenuID", IdAddToAlbum);
                ac->setText(fontMetrics().elidedText(album, Qt::ElideMiddle, 200));
                ac->setData(album);
                am->addAction(ac);
                break;
            }
        }
    }

    return am;
}

void ImagesView::popupDelDialog(const QStringList &paths)
{
    FileDeleteDialog *fdd = new FileDeleteDialog(paths);
    fdd->show();
}

LoadThread::LoadThread(const QString &album)
    :QThread(nullptr)
    , m_done(false)
    , m_album(album)
{

}

void LoadThread::run()
{
    auto infos = dApp->dbM->getInfosByAlbum(m_album);

    using namespace utils::image;
    for (auto info : infos) {
        if (m_done)
            return;
        ThumbnailListView::ItemInfo vi;
        vi.name = info.fileName;
        vi.path = info.filePath;
        vi.thumb = cutSquareImage(getThumbnail(vi.path, true));
        emit ready(vi);
    }
}

void LoadThread::setDone(bool done)
{
    m_done = done;
}
