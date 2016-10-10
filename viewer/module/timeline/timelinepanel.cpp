#include "timelinepanel.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/importframe.h"
#include "widgets/imagebutton.h"
#include "widgets/slider.h"
#include "frame/deletedialog.h"
#include <QPushButton>
#include <QFileDialog>
#include <QMimeData>
#include <QLabel>
#include <QDebug>
#include <QUrl>

namespace {

const int MIN_ICON_SIZE = 96;
const int ICON_MARGIN = 13;
const int SLIDER_WIDTH = 120;
//in order to make the toptoolbarContent align center, add leftMargin 82;
const int MARGIN_DIFF = 82;
//const int THUMBNAIL_MAX_SCALE_SIZE = 384;
const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

}  //namespace

using namespace Dtk::Widget;

TimelinePanel::TimelinePanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);

    initMainStackWidget();
    initStyleSheet();
    initPopupMenu();
    initConnection();

}

bool TimelinePanel::isMainPanel()
{
    return true;
}

QString TimelinePanel::moduleName()
{
    return "TimelinePanel";
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;
    tBottomContent->setStyleSheet(this->styleSheet());


    const int sizeScale = dApp->setter->value(SETTINGS_GROUP,
                                              SETTINGS_ICON_SCALE_KEY,
                                              QVariant(0)).toInt();
    const int iconSize = MIN_ICON_SIZE + sizeScale * 32;
    m_view->setIconSize(QSize(iconSize, iconSize));

    m_slider = new Slider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(3);
    m_slider->setValue(sizeScale);
    m_slider->setPageStep(1);
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        int newSize = MIN_ICON_SIZE + multiple * 32;
        m_view->setIconSize(QSize(newSize, newSize));
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ICON_SCALE_KEY,
                               QVariant(m_slider->value()));
    });

    m_countLabel = new QLabel;
    m_countLabel->setObjectName("CountLabel");
    updateBottomToolbarContent(dApp->databaseM->imageCount());

    ImageButton *ib = new ImageButton;
    ib->setToolTip(tr("Import"));
    ib->setNormalPic(":/images/resources/images/import_normal.png");
    ib->setHoverPic(":/images/resources/images/import_hover.png");
    ib->setPressPic(":/images/resources/images/import_press.png");
    connect(ib, &DImageButton::clicked, this, [=] {
        dApp->importer->showImportDialog();
    });

    QVBoxLayout* layout = new QVBoxLayout(tBottomContent);
    layout->setContentsMargins(0 ,0 ,0, 0);
    layout->setSpacing(0);

    QLabel* topDarkLine = new QLabel;
    topDarkLine->setFixedHeight(1);
    topDarkLine->setObjectName("BTopDarkLine");
    QLabel* topLightLine = new QLabel;
    topLightLine->setFixedHeight(1);
    topLightLine->setObjectName("BTopLightLine");

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(2, 0, 5, 0);
    hLayout->setSpacing(0);
    hLayout->addSpacing(6);
    hLayout->addWidget(ib);
    hLayout->addStretch(1);
    hLayout->addWidget(m_countLabel, 1, Qt::AlignCenter);
    hLayout->addWidget(m_slider, 1, Qt::AlignRight);
    hLayout->addSpacing(9);

    QLabel* bottomLightLine = new QLabel;
    bottomLightLine->setFixedHeight(1);
    bottomLightLine->setObjectName("BtmLightLine");

    layout->addWidget(topDarkLine);
    layout->addWidget(topLightLine);
    layout->addLayout(hLayout);
    layout->addWidget(bottomLightLine);

    return tBottomContent;
}

QWidget *TimelinePanel::toolbarTopLeftContent()
{
    QWidget *tTopleftContent = new QWidget;
    QLabel *label = new QLabel;
    label->setPixmap(QPixmap(":/images/logo/resources/images/logo/deepin_image_viewer_24.png"));
    QHBoxLayout *layout = new QHBoxLayout(tTopleftContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(ICON_MARGIN);
    layout->addWidget(label, 1, Qt::AlignLeft | Qt::AlignVCenter);

    return tTopleftContent;
}

QWidget *TimelinePanel::toolbarTopMiddleContent()
{
    QWidget *tTopMiddleContent = new QWidget;
    QLabel *timelineButton = new QLabel();

    timelineButton->setPixmap(QPixmap(":/images/resources/images/timeline_press.png"));

    ImageButton *albumButton = new ImageButton();
    albumButton->setNormalPic(":/images/resources/images/album_normal.png");
    albumButton->setHoverPic(":/images/resources/images/album_hover.png");
    connect(albumButton, &ImageButton::clicked, this, [=] {
        qDebug() << "Change to Album Panel...";
        emit dApp->signalM->gotoAlbumPanel();
    });
    albumButton->setToolTip(tr("Album"));

    // hide search button
    //    ImageButton *searchButton = new ImageButton();
    //    searchButton->setNormalPic(":/images/resources/images/search_normal_24px.png");
    //    searchButton->setHoverPic(":/images/resources/images/search_hover_24px.png");
    //    connect(searchButton, &ImageButton::clicked, this, [=] {
    //        qDebug() << "Change to Search Panel...";
    //        emit dApp->signalM->gotoSearchPanel();
    //    });

    //    searchButton->setToolTip("Search");

    QHBoxLayout *layout = new QHBoxLayout(tTopMiddleContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addSpacing(MARGIN_DIFF);
    layout->addWidget(timelineButton);
    layout->addWidget(albumButton);
    //    layout->addWidget(searchButton);

    return tTopMiddleContent;
}

QWidget *TimelinePanel::extensionPanelContent()
{
    return NULL;
}

void TimelinePanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QStringList files;
        for (QUrl url : urls) {
            const QString path = url.toLocalFile();
            if (QFileInfo(path).isDir()) {
                // Need popup AlbumCreate dialog
                emit dApp->signalM->importDir(path);
            }
            else if (utils::image::imageSupportRead(path)) {
                files << path;
            }
        }
        dApp->importer->importFiles(files);
    }
}

void TimelinePanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void TimelinePanel::showPanelEvent(ModulePanel *p)
{
    ModulePanel::showPanelEvent(p);
    emit dApp->signalM->showTopToolbar();
    emit dApp->signalM->showBottomToolbar();
    emit dApp->signalM->hideExtensionPanel(true);
    emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent(),
                                                   false);
}

void TimelinePanel::initConnection()
{
    connect(dApp->importer, &Importer::importProgressChanged,
            this, [=] (double v) {
        if (v == 1) {
            auto infos = dApp->databaseM->getAllImageInfos();
            for (auto info : infos) {
                m_view->onImageInserted(info);
            }
            onImageCountChanged(dApp->databaseM->imageCount());
        }
    });
    connect(dApp->signalM, &SignalManager::imagesInserted, this, [=] {
        onImageCountChanged(dApp->databaseM->imageCount());
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved, this, [=] {
        onImageCountChanged(dApp->databaseM->imageCount());
    });
    connect(dApp->signalM, &SignalManager::gotoTimelinePanel, this, [=] {
        m_view->setMultiSelection(false);

        emit dApp->signalM->gotoPanel(this);
    });
}

void TimelinePanel::initPopupMenu()
{
    m_popupMenu = new PopupMenuManager(this);
    updateMenuContents();
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &TimelinePanel::onMenuItemClicked);
}

void TimelinePanel::initMainStackWidget()
{
    initImagesView();

    ImportFrame *frame = new ImportFrame(this);
    frame->setButtonText(tr("Import"));
    frame->setTitle(tr("Import or drag image to timeline"));
    connect(frame, &ImportFrame::clicked, this, [=] {
        dApp->importer->showImportDialog();
    });

    m_mainStack = new QStackedWidget;
    m_mainStack->setContentsMargins(0, 0, 0, 0);
    m_mainStack->addWidget(frame);
    m_mainStack->addWidget(m_view);
    //show import frame if no images in database
    m_mainStack->setCurrentIndex(dApp->databaseM->imageCount() > 0 ? 1 : 0);

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStack);
}

void TimelinePanel::initImagesView()
{
    m_view = new TimelineImageView;
    m_view->setAcceptDrops(true);
    connect(m_view, &TimelineImageView::showMenuRequested, this, [=] {
        updateMenuContents();
        m_popupMenu->showMenu();
    });
    connect(m_view, &TimelineImageView::updateMenuRequested,
            this, &TimelinePanel::updateMenuContents);
    connect(m_view, &TimelineImageView::viewImage,
            this, [=] (const QString &path, const QStringList &paths){
        SignalManager::ViewInfo vinfo;
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths;
        emit dApp->signalM->viewImage(vinfo);
    });
}

void TimelinePanel::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/timeline.qss");
    if (!sf.open(QIODevice::ReadOnly)) {
        qWarning() << "Open style-sheet file error:" << sf.errorString();
        return;
    }

    this->setStyleSheet(QString(sf.readAll()));
    sf.close();
}

void TimelinePanel::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    m_rotateList.removeAll(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        m_view->updateThumbnails();
    }
}

void TimelinePanel::updateBottomToolbarContent(int count)
{
    if (count <= 1) {
        m_countLabel->setText(tr("%1 image").arg(count));
    }
    else {
        m_countLabel->setText(tr("%1 images").arg(count));
    }

//    int countHeight = utils::base::stringHeight(m_countLabel->font(),
//                                                m_countLabel->text());

//    m_countLabel->setMinimumHeight(countHeight);

    m_slider->setFixedWidth(count > 0 ? SLIDER_WIDTH : 0);
}

void TimelinePanel::updateMenuContents()
{
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void TimelinePanel::onMenuItemClicked(int menuId, const QString &text)
{
    QMap<QString, QString> images = m_view->selectedImages();
    if (images.isEmpty()) {
        return;
    }

    const QStringList nList = images.keys();
    const QStringList pList = images.values();
    const QStringList viewPaths = (pList.length() == 1) ?
                dApp->databaseM->getAllImagesPath() : pList;
    const QString cname = nList.first();
    const QString cpath = pList.first();

    SignalManager::ViewInfo vinfo;
    vinfo.inDatabase = true;
    vinfo.lastPanel = this;
    vinfo.path = cpath;
    vinfo.paths = viewPaths;

    switch (MenuItemId(menuId)) {
    case IdView:
        dApp->signalM->viewImage(vinfo);
        break;
    case IdFullScreen:
        vinfo.fullScreen = true;
        dApp->signalM->viewImage(vinfo);
        break;
    case IdStartSlideShow:
        dApp->signalM->startSlideShow(this, viewPaths, cpath);
        break;
    case IdAddToAlbum: {
        const QString album = text.split(SHORTCUT_SPLIT_FLAG).first();
        for (QString name : nList) {
            dApp->databaseM->insertImageIntoAlbum(
                album, name, utils::base::timeToString(imageInfo(name).time));
        }
        break;
    }
    case IdExport:
        dApp->exporter->exportImage(pList);
        break;
    case IdCopy:
        utils::base::copyImageToClipboard(pList);
        break;
    case IdMoveToTrash: {
        popupDelDialog(pList, nList);
        break;
    }
    case IdAddToFavorites:
        for(QString name : nList) {
        dApp->databaseM->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, name,
            utils::base::timeToString(imageInfo(cname).time));
        }
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        dApp->databaseM->removeImageFromAlbum(FAVORITES_ALBUM_NAME, cname);
        updateMenuContents();
        break;
    case IdRotateClockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = pList;
            for (QString path : pList) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, 90);
            }
        }
        break;
    case IdRotateCounterclockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = pList;
            for (QString path : pList) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, -90);
            }
        }
        break;
    case IdSetAsWallpaper:
        dApp->wpSetter->setWallpaper(cpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(cpath);
        break;
    case IdImageInfo:
        dApp->signalM->showImageInfo(cpath);
        break;
    default:
        break;
    }
}

void TimelinePanel::onImageCountChanged(int count)
{
    if (this->isVisible()) {
        updateBottomToolbarContent(count);
    }

    m_mainStack->setCurrentIndex(count > 0 ? 1 : 0);
}

QString TimelinePanel::createMenuContent()
{
    QMap<QString, QString> images = m_view->selectedImages();
    const QStringList pList = images.values();
    bool canSave = true;
    for (QString p : pList) {
        if (! utils::image::imageSupportSave(p)) {
            canSave = false;
            break;
        }
    }

    QJsonArray items;
    if (images.count() == 1) {
        items.append(createMenuItem(IdView, tr("View")));
        items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                    false, "F11"));
    }
    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"), false,
                                "F5"));
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

    items.append(createMenuItem(IdSeparator, "", true));

    if (images.count() == 1) {
        if (! dApp->databaseM->imageExistAlbum(images.firstKey(),
                                               FAVORITES_ALBUM_NAME))
            items.append(createMenuItem(IdAddToFavorites,
                         tr("Add to My favorites"), false, "Ctrl+K"));
        else
            items.append(createMenuItem(IdRemoveFromFavorites,
                         tr("Unfavorite"), false, "Ctrl+Shift+K"));
    } else {
        bool addToFavor = false;
        foreach (const QString &img, images) {
            if (!dApp->databaseM->imageExistAlbum(images.key(img),
                                                  FAVORITES_ALBUM_NAME)) {
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

    if (images.count() == 1) {
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

QJsonValue TimelinePanel::createMenuItem(const TimelinePanel::MenuItemId id,
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

QJsonObject TimelinePanel::createAlbumMenuObj()
{
    const QStringList albums = dApp->databaseM->getAlbumNameList();
    const QStringList selectNames = m_view->selectedImages().keys();

    QJsonArray items;
    if (! selectNames.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names =
                    dApp->databaseM->getImageNamesByAlbum(album);
            for (QString name : selectNames) {
                if (names.indexOf(name) == -1) {
                    items.append(createMenuItem(IdAddToAlbum, album));
                    break;
                }
            }
        }
    }

    QJsonObject contentObj;
    if (! items.isEmpty()) {
        contentObj[""] = QJsonValue(items);
    }

    return contentObj;
}

void TimelinePanel::popupDelDialog(const QStringList paths, const QStringList names) {
    DeleteDialog* delDialog = new DeleteDialog(paths, false, this);
    delDialog->show();
    delDialog->moveToCenter();
    connect(delDialog, &DeleteDialog::buttonClicked, [=](int index){
        if (index == 1) {
            dApp->databaseM->removeImages(names);
            utils::base::trashFiles(paths);
        }
    });

    connect(delDialog, &DeleteDialog::closed,
            delDialog, &DeleteDialog::deleteLater);

}

const DatabaseManager::ImageInfo TimelinePanel::imageInfo(
        const QString &name) const
{
    return dApp->databaseM->getImageInfoByName(name);
}
