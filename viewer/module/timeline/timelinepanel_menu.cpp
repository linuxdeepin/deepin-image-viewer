/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#include "timelinepanel.h"
#include "timelineframe.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/exporter.h"
#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/dialogs/filedeletedialog.h"
#include <QMenu>
#include <QShortcut>
#include <QStyleFactory>
#include <QtConcurrent>

#include <QPainter>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <iterator>

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorite";

const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";
const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

QString ss(const QString &text)
{
    QString str = dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString();
    str.replace(" ", "");
    return str;
}

enum MenuItemId {
    IdView,
    IdFullScreen,
    IdStartSlideShow,
    IdPrint,
    IdAddToAlbum,
    IdCopy,
    IdCopyToClipboard,
    IdMoveToTrash,
    IdRemoveFromTimeline,
    IdAddToFavorites,
    IdRemoveFromFavorites,
    IdRotateClockwise,
    IdRotateCounterclockwise,
    IdSetAsWallpaper,
    IdDisplayInFileManager,
    IdImageInfo,
};

}  // namespace

void TimelinePanel::initPopupMenu()
{
    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    connect(m_menu, &QMenu::triggered, this, &TimelinePanel::onMenuItemClicked);
    //Process num-keyboard's enter;
    QShortcut* sc = new QShortcut(QKeySequence("Enter"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        QStringList paths = m_frame->selectedPaths();
        paths.removeAll(QString(""));
        if (paths.isEmpty()) {
            return;
        }

        const QStringList viewPaths = (paths.length() == 1) ?
                    DBManager::instance()->getAllPaths() : paths;
        const QString dpath = paths.first();

        SignalManager::ViewInfo vinfo;
        vinfo.inDatabase = true;
        vinfo.lastPanel = this;
        vinfo.path = dpath;
        vinfo.paths = viewPaths;
        dApp->signalM->viewImage(vinfo);
    });
    sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        QStringList paths = m_frame->selectedPaths();
        paths.removeAll(QString(""));
        if (paths.isEmpty()) {
            return;
        }

        const QString dpath = paths.first();
        dApp->signalM->showImageInfo(dpath);
    });
}

void TimelinePanel::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(m_menu);
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    ac->setShortcut(QKeySequence(shortcut));
    m_menu->addAction(ac);
}

QMenu *TimelinePanel::createAlbumMenu()
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

void TimelinePanel::updateMenuContents()
{
    auto paths = m_frame->selectedPaths();
    if (paths.isEmpty())
        return;

    m_menu->clear();
    qDeleteAll(this->actions());

    bool canSave = false;
    if (paths.length() == 1) {
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
    if (paths.length() == 1)
        appendAction(IdCopyToClipboard, tr("Copy to clipboard"), ss("Copy to clipboard"));
    appendAction(IdMoveToTrash, tr("Throw to trash"), ss("Throw to trash"));
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
    if (paths.length() == 1)  {
        if (canSave) {
            appendAction(IdSetAsWallpaper,
                         tr("Set as wallpaper"), ss("Set as wallpaper"));
        }
        appendAction(IdDisplayInFileManager,
                     tr("Display in file manager"), ss("Display in file manager"));
    }
    appendAction(IdImageInfo, tr("Image info"), ss("Image info"));
}

void TimelinePanel::onMenuItemClicked(QAction *action)
{
    QStringList paths = m_frame->selectedPaths();
    paths.removeAll(QString(""));
    if (paths.isEmpty()) {
        return;
    }

    const QStringList viewPaths = (paths.length() == 1) ?
                DBManager::instance()->getAllPaths() : paths;
    const QString dpath = paths.first();

    SignalManager::ViewInfo vinfo;
    vinfo.inDatabase = true;
    vinfo.lastPanel = this;
    vinfo.path = dpath;
    vinfo.paths = viewPaths;

    const int id = action->property("MenuID").toInt();
    switch (id) {
    case IdView:
        dApp->signalM->viewImage(vinfo);
        break;
    case IdFullScreen:
        vinfo.fullScreen = true;
        dApp->signalM->viewImage(vinfo);
        break;
    case IdStartSlideShow:
        dApp->signalM->startSlideShow(vinfo);
        break;
    case IdPrint: {
        showPrintDialog(paths);
        break;
    }
    case IdAddToAlbum: {
        const QString album = action->data().toString();
        if (album != "Add to new album") {
            DBManager::instance()->insertIntoAlbum(album, paths);
        }else {
            dApp->signalM->createAlbum(paths);
        }
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdCopyToClipboard:
        utils::base::copyOneImageToClipboard(dpath);
        break;
    case IdMoveToTrash: {
        FileDeleteDialog *fdd = new FileDeleteDialog(paths);
        fdd->showInCenter(window());
        break;
    }
    case IdAddToFavorites:
        DBManager::instance()->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRemoveFromFavorites:
        DBManager::instance()->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRotateClockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, 90);
            }
        }
        break;
    case IdRotateCounterclockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, -90);
            }
        }
        break;
    case IdSetAsWallpaper:
        dApp->wpSetter->setWallpaper(dpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(dpath);
        break;
    case IdImageInfo:
        dApp->signalM->showImageInfo(dpath);
        break;
    default:
        break;
    }

    updateMenuContents();
}

void TimelinePanel::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    utils::image::generateThumbnail(path);
    m_rotateList.removeAll(path);
    m_frame->updateThumbnail(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        m_frame->updateScrollRange();
    }
}

void  TimelinePanel::showPrintDialog(const QStringList &paths) {
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    QPixmap img;

    QPrintDialog* printDialog = new QPrintDialog(&printer, this);
    printDialog->resize(400, 300);
    if (printDialog->exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QRectF drawRectF; QRect wRect;
        QList<QString>::const_iterator i;
        for(i = paths.begin(); i!= paths.end(); ++i){
            if (!img.load(*i)) {
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
