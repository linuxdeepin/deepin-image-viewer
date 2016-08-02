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

    QLabel* separatorLine = new QLabel;
    separatorLine->setObjectName("BtmSeparatorLine");
    separatorLine->setFixedHeight(1);
    QVBoxLayout* btmLayout = new QVBoxLayout(tBottomContent);
    btmLayout->setContentsMargins(0, 0, 0, 0);
    btmLayout->addWidget(separatorLine);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(14, 0, 14, 0);
    layout->setSpacing(0);

    if (m_targetAlbum.isEmpty()) {
        separatorLine->hide();
        const int sizeScale = dApp->setter->value(SETTINGS_GROUP,
                                                  SETTINGS_ICON_SCALE_KEY,
                                                  QVariant(0)).toInt();
        const int iconSize = MIN_ICON_SIZE + sizeScale * 32;
        m_view->setIconSize(QSize(iconSize, iconSize));

        m_slider = new Slider(Qt::Horizontal);
        m_slider->setMinimum(0);
        m_slider->setMaximum(3);
        m_slider->setValue(sizeScale);
        connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
            //            qDebug() << "Change the view size to: X" << multiple;
            int newSize = MIN_ICON_SIZE + multiple * 32;
            m_view->setIconSize(QSize(newSize, newSize));
            dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ICON_SCALE_KEY,
                                   QVariant(m_slider->value()));
        });

        m_countLabel = new QLabel;
        m_countLabel->setObjectName("CountLabel");

        updateBottomToolbarContent(dApp->databaseM->imageCount());


        layout->addStretch(1);
        layout->addWidget(m_countLabel, 1, Qt::AlignHCenter);
        layout->addWidget(m_slider, 1, Qt::AlignRight);
    }
    else {  // For import images to an album
        separatorLine->show();
        QVBoxLayout* titleLayout = new QVBoxLayout;
        titleLayout->setMargin(0);
        titleLayout->addStretch();
        QLabel *label = new QLabel;
        label->setObjectName("AddToAlbumTitle");
        label->setText(tr("Add to \"%1\" album").arg(m_targetAlbum));
        titleLayout->addWidget(label);
        titleLayout->addSpacing(7);
        titleLayout->addStretch();

        QVBoxLayout* cancelBtnLayout = new QVBoxLayout;
        cancelBtnLayout->setMargin(0);
        cancelBtnLayout->addStretch();
        QPushButton *cancelButton = new QPushButton(tr("Cancel"));
        cancelButton->setObjectName("AddToAlbumCancel");

        connect(cancelButton, &QPushButton::clicked, this, [=] {
            emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
            emit dApp->signalM->gotoAlbumPanel();
            emit dApp->signalM->imageAddedToAlbum();
        });
        cancelBtnLayout->addWidget(cancelButton);
        cancelBtnLayout->addSpacing(6);
        cancelBtnLayout->addStretch();

        QVBoxLayout* addBtnLayout = new QVBoxLayout;
        addBtnLayout->setMargin(0);
        addBtnLayout->addStretch();
        QPushButton *addButton = new QPushButton(tr("Add"));
        addButton->setObjectName("AddToAlbumAdd");
        addBtnLayout->addWidget(addButton);
        addBtnLayout->addSpacing(6);
        addBtnLayout->addStretch();
        connect(addButton, &QPushButton::clicked, this, [=] {
            QStringList images = m_view->selectedImages().keys();
            for (QString image : images) {
                // TODO improve performance
                auto info = dApp->databaseM->getImageInfoByName(image);
                dApp->databaseM->insertImageIntoAlbum(
                    m_targetAlbum, image, utils::base::timeToString(info.time));
            }

            emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
            emit dApp->signalM->gotoAlbumPanel();
            emit dApp->signalM->imageAddedToAlbum();
        });

        layout->addLayout(titleLayout);
        layout->addStretch(1);
        layout->addLayout(cancelBtnLayout);
        layout->addSpacing(10);
        layout->addLayout(addBtnLayout);
    }
    btmLayout->addLayout(layout);
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
    layout->setSpacing(20);
    layout->addStretch(1);
    layout->addWidget(timelineButton);
    layout->addWidget(albumButton);
    //    layout->addWidget(searchButton);
    layout->addStretch(1);

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
            else {
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
                                                ! m_targetAlbum.isEmpty());
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
    connect(dApp->signalM, &SignalManager::imageCountChanged,
            this, &TimelinePanel::onImageCountChanged);
    connect(dApp->signalM, &SignalManager::gotoTimelinePanel, this, [=] {
        m_targetAlbum = "";
        m_view->setTickable(false);
        m_view->clearSelection();
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

    connect(dApp->signalM, &SignalManager::addImageFromTimeline,
            this, [=] (const QString &targetAlbum) {
        m_targetAlbum = targetAlbum;
        m_view->setTickable(true);
        m_view->clearSelection();
        m_view->setMultiSelection(true);
        emit dApp->signalM->gotoPanel(this);
        emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent(), true);
    });

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

void TimelinePanel::updateBottomToolbarContent(int count)
{
    if (count <= 1) {
        m_countLabel->setText(tr("%1 image").arg(count));
    }
    else {
        m_countLabel->setText(tr("%1 images").arg(count));
    }

    int countHeight = utils::base::stringHeight(m_countLabel->font(),
                                                m_countLabel->text());
    m_countLabel->setMinimumHeight(countHeight);

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
    case IdMoveToTrash:
        for (QString name : nList) {
            dApp->databaseM->removeImage(name);
            utils::base::trashFile(images[name]);
        }
        break;
    case IdRemoveFromTimeline:
        for (QString name : nList) {
            dApp->databaseM->removeImage(name);
        }
        break;
    case IdAddToFavorites:
        dApp->databaseM->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, cname,
            utils::base::timeToString(imageInfo(cname).time));
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        dApp->databaseM->removeImageFromAlbum(FAVORITES_ALBUM_NAME, cname);
        updateMenuContents();
        break;
    case IdRotateClockwise:
        for (QString name : nList) {
            utils::image::rotate(images[name], 90);
            m_view->updateThumbnail(name);
        }
        break;
    case IdRotateCounterclockwise:
        for (QString name : nList) {
            utils::image::rotate(images[name], -90);
            m_view->updateThumbnail(name);
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
    if (! m_targetAlbum.isEmpty())
        return QString();

    QMap<QString, QString> images = m_view->selectedImages();
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

    items.append(createMenuItem(IdExport, tr("Export"), false, ""));
    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdMoveToTrash, tr("Throw to Trash"), false,
                                "Delete"));

    items.append(createMenuItem(IdSeparator, "", true));
    if (images.count() == 1) {
        if (! dApp->databaseM->imageExistAlbum(images.firstKey(),
                                               FAVORITES_ALBUM_NAME)) {
            items.append(createMenuItem(IdAddToFavorites,
                tr("Add to My favorites"), false, "Ctrl+K"));
        }
        else {
            items.append(createMenuItem(IdRemoveFromFavorites, tr("Unfavorite"),
                                        false, "Ctrl+Shift+K"));
        }
    }
    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"),
                                false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise,
        tr("Rotate counterclockwise"), false, "Ctrl+Shift+R"));

    items.append(createMenuItem(IdSeparator, "", true));

    //    items.append(createMenuItem(IdLabel, tr("Text tag")));
    if (images.count() == 1) {
        items.append(createMenuItem(IdSetAsWallpaper, tr("Set as wallpaper"),
                                    false, "Ctrl+F8"));
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

const DatabaseManager::ImageInfo TimelinePanel::imageInfo(
        const QString &name) const
{
    return dApp->databaseM->getImageInfoByName(name);
}
