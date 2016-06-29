#include "timelinepanel.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "controller/importer.h"
#include "controller/exporter.h"
#include "controller/popupmenumanager.h"
#include "controller/wallpapersetter.h"
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
//const int THUMBNAIL_MAX_SCALE_SIZE = 384;
const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

}  //namespace

using namespace Dtk::Widget;

TimelinePanel::TimelinePanel(QWidget *parent)
    : ModulePanel(parent),
      m_dbManager(DatabaseManager::instance())
{
    setAcceptDrops(true);

    initMainStackWidget();
    initStyleSheet();
    initPopupMenu();

    connect(m_sManager, &SignalManager::imageCountChanged,
        this, &TimelinePanel::onImageCountChanged/*, Qt::DirectConnection*/);
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;
    tBottomContent->setStyleSheet(this->styleSheet());

    QHBoxLayout *layout = new QHBoxLayout(tBottomContent);
    layout->setContentsMargins(14, 0, 14, 0);
    layout->setSpacing(0);

    if (m_targetAlbum.isEmpty()) {
        m_slider = new Slider(Qt::Horizontal);
        m_slider->setMinimum(0);
        m_slider->setMaximum(3);
        m_slider->setValue(0);
        m_slider->setFixedWidth(120);
        connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
            qDebug() << "Change the view size to: X" << multiple;
            int newSize = MIN_ICON_SIZE + multiple * 32;
            m_imagesView->setIconSize(QSize(newSize, newSize));
        });

        m_countLabel = new QLabel;
        m_countLabel->setObjectName("CountLabel");

        updateBottomToolbarContent(m_dbManager->imageCount());

        layout->addStretch(1);
        layout->addWidget(m_countLabel, 1, Qt::AlignHCenter);
        layout->addWidget(m_slider, 1, Qt::AlignRight);
    }
    else {  // For import images to an album
        QLabel *label = new QLabel;
        label->setObjectName("AddToAlbumTitle");
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setText(tr("Add to \"%1\" album").arg(m_targetAlbum));

        QPushButton *cancelButton = new QPushButton(tr("Cancel"));
        cancelButton->setObjectName("AddToAlbumCancel");
        connect(cancelButton, &QPushButton::clicked, this, [=] {
            m_targetAlbum = "";
            m_mainStackWidget->setCurrentWidget(m_imagesView);
            emit m_sManager->updateBottomToolbarContent(toolbarBottomContent());
            emit m_sManager->gotoAlbumPanel();
        });

        QPushButton *addButton = new QPushButton(tr("Add"));
        addButton->setObjectName("AddToAlbumAdd");
        connect(addButton, &QPushButton::clicked, this, [=] {
            QStringList images = m_selectionView->selectedImages().keys();
            for (QString image : images) {
                // TODO improve performance
                DatabaseManager::ImageInfo info
                        = m_dbManager->getImageInfoByName(image);
                m_dbManager->insertImageIntoAlbum(m_targetAlbum, image,
                    utils::base::timeToString(info.time));
            }

            m_targetAlbum = "";
            m_mainStackWidget->setCurrentWidget(m_imagesView);
            emit m_sManager->updateBottomToolbarContent(toolbarBottomContent());
            emit m_sManager->gotoAlbumPanel();
        });

        layout->addWidget(label);
        layout->addStretch(1);
        layout->addWidget(cancelButton);
        layout->addSpacing(10);
        layout->addWidget(addButton);
    }

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
        emit m_sManager->gotoAlbumPanel();
    });
    albumButton->setToolTip("Album");

    // hide search button
//    ImageButton *searchButton = new ImageButton();
//    searchButton->setNormalPic(":/images/resources/images/search_normal_24px.png");
//    searchButton->setHoverPic(":/images/resources/images/search_hover_24px.png");
//    connect(searchButton, &ImageButton::clicked, this, [=] {
//        qDebug() << "Change to Search Panel...";
//        emit m_sManager->gotoSearchPanel();
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
        for (QUrl url : urls) {
            const QString path = url.toLocalFile();
            if (QFileInfo(path).isDir()) {
                // Need popup AlbumCreate dialog
                emit m_sManager->importDir(path);
            }
            else {
                if (utils::image::imageIsSupport(path)) {
                    Importer::instance()->importSingleFile(path);
                }
            }
        }
    }
}

void TimelinePanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void TimelinePanel::initPopupMenu()
{
    m_popupMenu = new PopupMenuManager(this);
    updateMenuContents();
    connect(m_imagesView, &TimelineImageView::customContextMenuRequested,
            this, [=] {
        updateMenuContents();
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &TimelinePanel::onMenuItemClicked);
}

void TimelinePanel::initMainStackWidget()
{
    initImagesView();
    initSelectionView();

    m_mainStackWidget = new QStackedWidget;
    m_mainStackWidget->setContentsMargins(0, 0, 0, 0);
    m_mainStackWidget->addWidget(new ImportFrame(this));
    m_mainStackWidget->addWidget(m_imagesView);
    m_mainStackWidget->addWidget(m_selectionView);
    //show import frame if no images in database
    m_mainStackWidget->setCurrentIndex(m_dbManager->imageCount() > 0 ? 1 : 0);

    connect(m_sManager, &SignalManager::selectImageFromTimeline,
            this, [=] (const QString &targetAlbum) {
        m_targetAlbum = targetAlbum;
        m_mainStackWidget->setCurrentWidget(m_selectionView);
        emit m_sManager->gotoPanel(this);
        emit m_sManager->updateBottomToolbarContent(toolbarBottomContent(), true);
    });

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStackWidget);
}

void TimelinePanel::initImagesView()
{
    m_imagesView = new TimelineImageView;
    m_imagesView->setAcceptDrops(true);
}

void TimelinePanel::initSelectionView()
{
    m_selectionView = new TimelineImageView(true);
    m_selectionView->setAcceptDrops(false);
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
    if (! this->isVisible()) {
        return;
    }

    if (count <= 1) {
        m_countLabel->setText(tr("%1 image").arg(count));
    }
    else {
        m_countLabel->setText(tr("%1 images").arg(count));
    }

    m_slider->setFixedWidth(count > 0 ? 120 : 1);
}

void TimelinePanel::updateMenuContents()
{
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void TimelinePanel::onMenuItemClicked(int menuId, const QString &text)
{
    QMap<QString, QString> images = m_imagesView->selectedImages();
    if (images.isEmpty()) {
        return;
    }

    const QString cname = images.keys().first();
    const QString cpath = images.values().first();
    switch (MenuItemId(menuId)) {
    case IdView:
        m_sManager->viewImage(cpath);
        break;
    case IdFullScreen:
        m_sManager->viewImage(cpath);
        m_sManager->fullScreen(cpath);
        break;
    case IdStartSlideShow:
        m_sManager->viewImage(cpath);
        m_sManager->startSlideShow(cpath);
        break;

    case IdAddToAlbum: {
        const QString album = text.split(SHORTCUT_SPLIT_FLAG).first();
        for (QString name : images.keys()) {
            m_dbManager->insertImageIntoAlbum(album, name,
                utils::base::timeToString(imageInfo(cname).time));
        }
        break;
    }
    case IdExport:
        Exporter::instance()->exportImage(m_imagesView->selectedImages().values());
        break;
    case IdCopy:
        utils::base::copyImageToClipboard(images.values());
        break;
    case IdMoveToTrash:
        for (QString name : images.keys()) {
            m_dbManager->removeImage(name);
            utils::base::trashFile(images[name]);
        }
        break;
    case IdRemoveFromTimeline:
        for (QString name : images.keys()) {
            m_dbManager->removeImage(name);
        }
        break;
//    case IdEdit:
//        m_sManager->editImage(cpath);
//        break;
    case IdAddToFavorites:
        for (QString name : images.keys()) {
            m_dbManager->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, name,
                utils::base::timeToString(imageInfo(name).time));
        }
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        for (QString name : images.keys()) {
            m_dbManager->removeImageFromAlbum(FAVORITES_ALBUM_NAME, name);
        }
        updateMenuContents();
        break;
    case IdRotateClockwise:
        for (QString name : images.keys()) {
            utils::image::rotate(images[name], 90);
            m_imagesView->updateThumbnail(name);
            m_selectionView->updateThumbnail(name);
        }
        break;
    case IdRotateCounterclockwise:
        for (QString name : images.keys()) {
            utils::image::rotate(images[name], -90);
            m_imagesView->updateThumbnail(name);
            m_selectionView->updateThumbnail(name);
        }
        break;
//    case IdLabel:
//        break;
    case IdSetAsWallpaper:
        WallpaperSetter::instance()->setWallpaper(cpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(cpath);
        break;
    case IdImageInfo:
        m_sManager->showImageInfo(cpath);
        break;
    default:
        break;
    }
}

void TimelinePanel::onImageCountChanged(int count)
{
    updateBottomToolbarContent(count);
    m_mainStackWidget->setCurrentIndex(count > 0 ? 1 : 0);
}

QString TimelinePanel::createMenuContent()
{
    QMap<QString, QString> images = m_imagesView->selectedImages();
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
    items.append(createMenuItem(IdMoveToTrash, tr("Throw to trash")));

    items.append(createMenuItem(IdSeparator, "", true));
//    items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
    for (QString name : images.keys()) {
        if (! m_dbManager->imageExistAlbum(name, FAVORITES_ALBUM_NAME)) {
            items.append(createMenuItem(IdAddToFavorites,
                tr("Add to My favorites"), false, "Ctrl+K"));
            break;
        }
        else {
            items.append(createMenuItem(IdRemoveFromFavorites,
                tr("Unfavorite"), false, "Ctrl+Shift+K"));
            break;
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
    const QStringList albums = m_dbManager->getAlbumNameList();
    const QStringList selectNames = m_imagesView->selectedImages().keys();

    QJsonArray items;
    if (! selectNames.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names = m_dbManager->getImageNamesByAlbum(album);
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

const DatabaseManager::ImageInfo TimelinePanel::imageInfo(const QString &name) const
{
    return m_dbManager->getImageInfoByName(name);
}
