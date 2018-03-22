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
#include "imagesview.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"

#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/importframe.h"
#include "widgets/dialogs/filedeletedialog.h"
#include "snifferimageformat.h"

#include <QDebug>
#include <QFileInfo>
#include <QMenu>
#include <QShortcut>
#include <QStandardItem>
#include <QStackedWidget>
#include <QStyleFactory>
#include <QtConcurrent>
#include <QJsonArray>
#include <QJsonDocument>
#include <math.h>

#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

namespace {

const int TOP_TOOLBAR_HEIGHT = 39;
const QString FAVORITES_ALBUM_NAME = "My favorite";
const QString RECENT_IMPORTED_ALBUM = "Recent imported";
const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";

QString ss(const QString &text)
{
    QString str = dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString();
    str.replace(" ", "");
    return str;
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
    : QStackedWidget(parent)
{
    setObjectName("ImagesView");

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

    // Update tip's info
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

    addWidget(m_view);

    connect(m_view, &ThumbnailListView::changeItemSize,
            this, &ImagesView::changeItemSize);
//    connect(m_view, &ThumbnailListView::clicked,
//            this, &ImagesView::updateMenuContents);
    // When user use cursor to drag to select area
    // The signal will be triggered frequently
    // Use timer to reset it
    QTimer *t = new QTimer(this);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, this, &ImagesView::updateMenuContents);
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [=] (const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous)
        if (current.isValid())
            t->start(200);
    });
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
}

void ImagesView::initTopTips()
{
    m_topTips = new TopAlbumTips(this);
    m_topTips->raise();
}

void ImagesView::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(m_menu);
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    ac->setShortcut(QKeySequence(shortcut));
    m_menu->addAction(ac);
}

const QStringList ImagesView::albumPaths()
{
    return  DBManager::instance()->getPathsByAlbum(m_album);
}

void ImagesView::insertItem(const DBImgInfo &info, bool update)
{
    using namespace utils::image;
    ThumbnailListView::ItemInfo vi;
    vi.name = info.fileName;
    vi.path = info.filePath;
    vi.thumb = cutSquareImage(getThumbnail(vi.path, true));

    m_view->insertItem(vi);
    // Update tip's info
    m_topTips->setAlbum(m_album);

    if (update) {
        m_view->update();
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

    // Update tip's info
    m_topTips->setAlbum(m_album);

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
    bool canSave = false;

    if (selectedCount == 1) {
        appendAction(IdView, tr("View"), ss("View"));
        appendAction(IdFullScreen, tr("Fullscreen"), ss("Fullscreen"));

        auto supportPath = std::find_if_not(paths.cbegin(), paths.cend(),
                                            utils::image::imageSupportSave);
        canSave = supportPath == paths.cend();
    }
    appendAction(IdStartSlideShow, tr("Slide show"), ss("Slide show"));
    appendAction(IdPrint, tr("Print"), ss("Print"));
    QMenu *am = createAlbumMenu();
    if (am) {
        m_menu->addMenu(am);
    }
    m_menu->addSeparator();
    /**************************************************************************/
    appendAction(IdCopy, tr("Copy"), ss("Copy"));
    if (selectedCount == 1)
        appendAction(IdCopyToClipboard, tr("Copy to clipboard"), ss("Copy to clipboard"));
    appendAction(IdMoveToTrash, tr("Throw to trash"), ss("Throw to trash"));
    appendAction(IdRemoveFromAlbum, tr("Remove from album"), ss("Remove from album"));
    m_menu->addSeparator();
    /**************************************************************************/
    appendAction(IdAddToFavorites,
                 tr("Favorite"), ss("Favorite"));
    appendAction(IdRemoveFromFavorites, tr("Unfavorite"),
                 ss("Unfavorite"));
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
        showPrintDialog(paths);
        break;
    }
    case IdAddToAlbum: {
        const QString album = action->data().toString();
        if (album != "Add to new album") {
           DBManager::instance()->insertIntoAlbum(album, paths);
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
        DBManager::instance()->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRemoveFromFavorites:
        DBManager::instance()->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRemoveFromAlbum:
        m_view->removeItems(paths);
        DBManager::instance()->removeFromAlbum(m_album, paths);
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
    utils::image::generateThumbnail(path);
    m_rotateList.removeAll(path);
    m_view->updateThumbnail(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        emit rotateFinished();
    }
}

bool ImagesView::allInAlbum(const QStringList &paths, const QString &album)
{
    const QStringList pl = DBManager::instance()->getPathsByAlbum(album);
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
    m_topTips->move(0, TOP_TOOLBAR_HEIGHT);
    m_topTips->resize(width(), m_topTips->height());
    const int lm = - m_view->hOffset();
    m_topTips->setLeftMargin(lm);
    m_topTips->setVisible(true);
    m_topTips->raise();
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
    return m_view->selectedPaths();
}

void ImagesView::resizeEvent(QResizeEvent *e)
{
    QStackedWidget::resizeEvent(e);
    updateTopTipsRect();
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

    // Update tip's info
    m_topTips->setAlbum(m_album);
    updateContent();
    updateMenuContents();
}

void ImagesView::initPopupMenu()
{
    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    connect(m_menu, &QMenu::triggered, this, &ImagesView::onMenuItemClicked);
    connect(this, &ImagesView::rotateFinished,
            this, &ImagesView::updateMenuContents);
    QShortcut* sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        QStringList paths = selectedPaths();
        paths.removeAll(QString(""));
        if (paths.isEmpty()) {
            return;
        }

        const QString path = paths.first();
        dApp->signalM->showImageInfo(path);
     });
    //Process num-keyboard's enter;
    sc = new QShortcut(QKeySequence("Enter"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        QStringList paths = selectedPaths();
        paths.removeAll(QString(""));
        if (paths.isEmpty()) {
            return;
        }

        const QStringList viewPaths = (paths.length() == 1) ? albumPaths() : paths;
        const QString path = paths.first();
        emit viewImage(path, viewPaths);
    });
}

void ImagesView::initContent()
{
    m_importFrame = new ImportFrame(this);

    m_importFrame->setButtonText(tr("Add"));
    m_importFrame->setTitle(tr("You can add sync directory or drag and drop  images to timeline"));
    connect(m_importFrame, &ImportFrame::clicked, this, [=] {
         Importer::instance()->showImportDialog(m_album);
    });

    initListView();
    initTopTips();

    addWidget(m_importFrame);
}

void ImagesView::updateContent()
{
    if (DBManager::instance()->getImgsCountByAlbum(m_album) < 1) {
        setCurrentWidget(m_importFrame);
    }
    else {
        setCurrentWidget(m_view);
    }
}

QMenu *ImagesView::createAlbumMenu()
{
    QMenu *am = new QMenu(tr("Add to album"));
    am->setStyle(QStyleFactory::create("dlight"));
    QStringList albums = DBManager::instance()->getAllAlbumNames();
    albums.removeAll(FAVORITES_ALBUM_NAME);

    QAction *ac = new QAction(am);
    ac->setProperty("MenuID", IdAddToAlbum);
    ac->setText(tr("Add to new album"));
    ac->setData(QString("Add to new album"));
    am->addAction(ac);
    am->addSeparator();
    for (QString album : albums) {
        QAction *ac = new QAction(am);
        ac->setProperty("MenuID", IdAddToAlbum);
        ac->setText(fontMetrics().elidedText(QString(album).replace("&", "&&"), Qt::ElideMiddle, 200));
        ac->setData(album);
        am->addAction(ac);
    }

    return am;
}

void ImagesView::popupDelDialog(const QStringList &paths)
{
    FileDeleteDialog *fdd = new FileDeleteDialog(paths);
    fdd->showInCenter(window());
}

void  ImagesView::showPrintDialog(const QStringList &paths) {
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    QPixmap img;

    QPrintDialog* printDialog = new QPrintDialog(&printer, this);
    printDialog->resize(400, 300);
    if (printDialog->exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QList<QString>::const_iterator i;
        QRectF drawRectF; QRect wRect;
        for(i = paths.begin(); i!= paths.end(); ++i){
            const QString format = DetectImageFormat(*i);
            if (!img.load(*i, format.toLatin1())) {
                qDebug() << "img load failed" << *i;
                continue;
            }
            wRect = printer.pageRect();
            if (img.width() > wRect.width() || img.height() > wRect.height()) {
                img = img.scaled(wRect.size(), Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
            }
            drawRectF = QRectF(qreal(wRect.width() - img.width())/2,
                               qreal(wRect.height() - img.height())/2,
                              img.width(), img.height());
            painter.drawPixmap(drawRectF.x(), drawRectF.y(), img.width(),
                               img.height(), img);
            if (i != paths.end() - 1)
                printer.newPage();
        }
        painter.end();
        qDebug() << "print succeed!";
        return;
    }

    QObject::connect(printDialog, &QPrintDialog::finished, printDialog,
            &QPrintDialog::deleteLater);

    qDebug() << "print failed!";
}

LoadThread::LoadThread(const QString &album)
    :QThread(nullptr)
    , m_done(false)
    , m_album(album)
{

}

void LoadThread::run()
{
    auto infos = DBManager::instance()->getInfosByAlbum(m_album);

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
