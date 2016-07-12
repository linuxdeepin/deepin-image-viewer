#include "albumpanel.h"
#include "createalbumdialog.h"
#include "importdirdialog.h"
#include "controller/configsetter.h"
#include "controller/databasemanager.h"
#include "controller/importer.h"
#include "utils/imageutils.h"
#include "widgets/importframe.h"
#include "widgets/imagebutton.h"
#include "widgets/slider.h"
#include <QFileInfo>
#include <QDropEvent>
#include <QPushButton>
#include <QPointer>
#include <QDebug>

namespace {

const int MIN_ICON_SIZE = 96;
const int ICON_MARGIN = 13;

const QString SETTINGS_GROUP = "ALBUMPANEL";
const QString SETTINGS_ALBUM_ICON_SCALE_KEY = "AlbumIconScale";
const QString SETTINGS_IMAGE_ICON_SCALE_KEY = "ImageIconScale";

}   // namespace

AlbumPanel::AlbumPanel(QWidget *parent)
    : ModulePanel(parent),
      m_setter(ConfigSetter::instance()),
      m_dbManager(DatabaseManager::instance()),
      m_sManager(SignalManager::instance())
{
    setAcceptDrops(true);
    initMainStackWidget();
    initStyleSheet();

    connect(m_sManager, &SignalManager::importDir,
            this, &AlbumPanel::showImportDirDialog);
    connect(m_sManager, &SignalManager::imageCountChanged,
            this, &AlbumPanel::onImageCountChanged/*, Qt::DirectConnection*/);
}


QWidget *AlbumPanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;
    tBottomContent->setStyleSheet(this->styleSheet());

    m_slider = new Slider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(3);
    m_slider->setValue(0);
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        if (m_stackWidget->currentWidget() == m_imagesView) {
            int newSize = MIN_ICON_SIZE + multiple * 32;
            m_imagesView->setIconSize(QSize(newSize, newSize));
            m_setter->setValue(SETTINGS_GROUP, SETTINGS_IMAGE_ICON_SCALE_KEY,
                               QVariant(m_slider->value()));
        }
        else {
            m_albumsView->setItemSizeMultiple(multiple);
            m_setter->setValue(SETTINGS_GROUP, SETTINGS_ALBUM_ICON_SCALE_KEY,
                               QVariant(m_slider->value()));
        }
    });

    m_countLabel = new QLabel;
    m_countLabel->setAlignment(Qt::AlignLeft);
    m_countLabel->setObjectName("CountLabel");

    updateAlbumCount();

    connect(m_sManager, &SignalManager::selectImageFromTimeline, this, [=] {
        emit m_sManager->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit m_sManager->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    });

    QHBoxLayout *layout = new QHBoxLayout(tBottomContent);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->addStretch(1);
    layout->addWidget(m_countLabel, 1, Qt::AlignCenter);
    layout->addWidget(m_slider, 1, Qt::AlignRight);
    layout->addSpacing(9);

    return tBottomContent;
}

QWidget *AlbumPanel::toolbarTopLeftContent()
{
    QWidget *tTopleftContent = new QWidget;
    tTopleftContent->setStyleSheet(this->styleSheet());

    QHBoxLayout *layout = new QHBoxLayout(tTopleftContent);
    layout->setContentsMargins(0, 0, 0, 0);

    if (m_stackWidget->currentWidget() == m_imagesView) {
        layout->addSpacing(9);
        ImageButton *returnButton = new ImageButton();

        returnButton->setNormalPic(":/images/resources/images/return_normal.png");
        returnButton->setHoverPic(":/images/resources/images/return_hover.png");
        returnButton->setPressPic(":/images/resources/images/return_press.png");
        connect(returnButton, &ImageButton::clicked, this, [=] {
            m_stackWidget->setCurrentWidget(m_albumsView);
            // Make sure top toolbar content still show as album content
            // during adding images from timeline
            emit m_sManager->gotoPanel(this);
        });
        returnButton->setToolTip(tr("Back"));

        layout->addWidget(returnButton);
        layout->addStretch(1);
    }
    else {
        layout->addSpacing(ICON_MARGIN);
        QLabel *icon = new QLabel;
        // TODO update icon path
        icon->setPixmap(QPixmap(":/images/logo/resources/images/logo/deepin_image_viewer_24.png"));

        layout->addWidget(icon);
        layout->addStretch(1);
    }

    return tTopleftContent;
}

QWidget *AlbumPanel::toolbarTopMiddleContent()
{
    QWidget *tTopMiddleContent = new QWidget;

    ImageButton *timelineButton = new ImageButton();
    timelineButton->setNormalPic(":/images/resources/images/timeline_normal.png");
    timelineButton->setHoverPic(":/images/resources/images/timeline_hover.png");
    connect(timelineButton, &ImageButton::clicked, this, [=] {
        qDebug() << "Change to Timeline Panel...";
        emit m_sManager->backToMainWindow();
    });
    timelineButton->setToolTip("Timeline");

    QLabel *albumLabel = new QLabel();
    albumLabel->setPixmap(QPixmap(":/images/resources/images/album_active.png"));


    // hide search button
//    ImageButton *searchButton = new ImageButton();
//    searchButton->setNormalPic(":/images/resources/images/search_normal_24px.png");
//    searchButton->setHoverPic(":/images/resources/images/search_hover_24px.png");
//    searchButton->setPressPic(":/images/resources/images/search_press_24px.png");
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
    layout->addWidget(albumLabel);
//    layout->addWidget(searchButton);
    layout->addStretch(1);

    return tTopMiddleContent;
}

QWidget *AlbumPanel::extensionPanelContent()
{
    return NULL;
}

void AlbumPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    const QString currentAlbum = m_imagesView->getCurrentAlbum();
    for (QUrl url : urls) {
        const QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            if (m_stackWidget->currentWidget() == m_albumsView) {
                showImportDirDialog(path);
            }
            else {
                Importer::instance()->importFromPath(path, currentAlbum);
            }
        }
        else {
            if (m_stackWidget->currentWidget() == m_albumsView) {
                Importer::instance()->importSingleFile(path);
            }
            else {
                Importer::instance()->importSingleFile(path, currentAlbum);
            }
        }
    }
}

void AlbumPanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void AlbumPanel::initMainStackWidget()
{
    initImagesView();
    initAlbumsView();

    m_stackWidget = new QStackedWidget;
    m_stackWidget->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->addWidget(new ImportFrame(this));
    m_stackWidget->addWidget(m_albumsView);
    m_stackWidget->addWidget(m_imagesView);
    //show import frame if no images in database
    m_stackWidget->setCurrentIndex(m_dbManager->imageCount() > 0 ? 1 : 0);
    connect(m_stackWidget, &QStackedWidget::currentChanged, this, [=] {
        updateImagesCount();
        updateAlbumCount();
        emit m_sManager->updateTopToolbarLeftContent(
                    toolbarTopLeftContent());
    });

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stackWidget);
}

void AlbumPanel::initAlbumsView()
{
    m_albumsView = new AlbumsView(this);
    m_albumsView->updateView();
    connect(m_albumsView, &AlbumsView::openAlbum,
            this, &AlbumPanel::onOpenAlbum);
    connect(m_albumsView, &AlbumsView::albumCreated,
            this, &AlbumPanel::updateAlbumCount,
            Qt::QueuedConnection);
    connect(m_albumsView, &AlbumsView::albumRemoved,
            this, &AlbumPanel::updateAlbumCount);
}

void AlbumPanel::initImagesView()
{
    m_imagesView = new ImagesView(this);
    connect(m_sManager, &SignalManager::insertIntoAlbum,
            this, &AlbumPanel::onInsertIntoAlbum, Qt::QueuedConnection);
    connect(m_sManager, &SignalManager::removeFromAlbum,
            this, &AlbumPanel::updateImagesCount);
}

void AlbumPanel::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/album.qss");
    if (!sf.open(QIODevice::ReadOnly)) {
        qWarning() << "Open style-sheet file error:" << sf.errorString();
        return;
    }

    setStyleSheet(QString(sf.readAll()));
    sf.close();
}

void AlbumPanel::updateImagesCount()
{
    if (m_stackWidget->currentWidget() != m_imagesView)
        return;

    const int count = m_imagesView->count();
    QString text = QString::number(count) + " " +
            (count <= 1 ? tr("Image") : tr("Images"));
    m_countLabel->setText(text);

    m_slider->setValue(m_setter->value(SETTINGS_GROUP,
                                       SETTINGS_IMAGE_ICON_SCALE_KEY,
                                       QVariant(0)).toInt());

    //set width to 1px for layout center
    m_slider->setFixedWidth(count > 0 ? 110 : 1);
}

void AlbumPanel::updateAlbumCount()
{
    if (m_stackWidget->currentWidget() != m_albumsView)
        return;

    const int count = m_dbManager->albumsCount();
    QString text = QString::number(count) + " " +
            (count <= 1 ? tr("Album") : tr("Albums"));
    m_countLabel->setText(text);

    m_slider->setValue(m_setter->value(SETTINGS_GROUP,
                                       SETTINGS_ALBUM_ICON_SCALE_KEY,
                                       QVariant(0)).toInt());

    //set width to 1px for layout center
    m_slider->setFixedWidth(count > 0 ? 110 : 1);
}

void AlbumPanel::showCreateDialog()
{
    if (! parentWidget()) {
        return;
    }
    CreateAlbumDialog *d = new CreateAlbumDialog(this, parentWidget());
    connect(d, &CreateAlbumDialog::closed,
            d, &CreateAlbumDialog::deleteLater);
    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
            (parentWidget()->height() - d->height()) / 2 + p.y());
    d->show();
}

void AlbumPanel::showImportDirDialog(const QString &dir)
{
    if (! parentWidget() || utils::image::getImagesInfo(dir).isEmpty()) {
        return;
    }
    ImportDirDialog *d = new ImportDirDialog(this, parentWidget());
    connect(d, &ImportDirDialog::albumCreated,
            m_albumsView, &AlbumsView::updateView);
    connect(d, &ImportDirDialog::closed,
            d, &ImportDirDialog::deleteLater);
    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
            (parentWidget()->height() - d->height()) / 2 + p.y());
    d->import(dir);
}

void AlbumPanel::onImageCountChanged(int count)
{
    if (! isVisible())
        return;

    if (count > 0 && m_stackWidget->currentIndex() == 0) {
        m_stackWidget->setCurrentIndex(1);
    }
    else if (count == 0 && m_stackWidget->currentIndex() == 1) {
        m_stackWidget->setCurrentIndex(0);
    }
    updateImagesCount();
}

void AlbumPanel::onInsertIntoAlbum(const DatabaseManager::ImageInfo info)
{
    if (m_imagesView->isVisible()
            && info.albums.contains(m_imagesView->getCurrentAlbum())) {
        m_imagesView->insertItem(info);
        updateImagesCount();
    }
}

void AlbumPanel::onOpenAlbum(const QString &album)
{
    qDebug() << "Open Album : " << album;
    m_currentAlbum = album;
    m_stackWidget->setCurrentIndex(2);
    const int multiple = m_setter->value(SETTINGS_GROUP,
                                         SETTINGS_IMAGE_ICON_SCALE_KEY,
                                         QVariant(0)).toInt();
    int newSize = MIN_ICON_SIZE + multiple * 32;
    m_imagesView->setIconSize(QSize(newSize, newSize));
    m_imagesView->setAlbum(album);
    TIMER_SINGLESHOT(1000, {updateImagesCount();}, this);
}

void AlbumPanel::onCreateAlbum()
{
    if (this->isVisible())
        m_albumsView->createAlbum();
    else
        showCreateDialog();
}

void AlbumPanel::showEvent(QShowEvent *e)
{
    // Make sure BottomContent have been init
    emit m_sManager->updateBottomToolbarContent(toolbarBottomContent());
    onImageCountChanged(m_dbManager->imageCount());
    ModulePanel::showEvent(e);
}
