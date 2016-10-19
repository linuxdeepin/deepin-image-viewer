#include "albumpanel.h"
#include "application.h"
#include "createalbumdialog.h"
#include "importdirdialog.h"
#include "controller/configsetter.h"
#include "controller/databasemanager.h"
#include "controller/importer.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"
#include "widgets/importframe.h"
#include "widgets/slider.h"

//#include <dthememanager.h>

#include <QDebug>
#include <QDropEvent>
#include <QFileInfo>
#include <QPointer>
#include <QPushButton>

DWIDGET_USE_NAMESPACE

namespace {

const int MIN_ICON_SIZE = 96;
const int ICON_MARGIN = 13;
const int SLIDER_WIDTH = 120;
const int MARGIN_DIFF = 82;
const QString RECENT_IMPORT_ALBUM = "Recent imported";
const QString SETTINGS_GROUP = "ALBUMPANEL";
const QString SETTINGS_ALBUM_ICON_SCALE_KEY = "AlbumIconScale";
const QString SETTINGS_IMAGE_ICON_SCALE_KEY = "ImageIconScale";

}   // namespace

AlbumPanel::AlbumPanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);
    initMainStackWidget();
    initStyleSheet();
    initConnection();
}

bool AlbumPanel::isMainPanel()
{
    return true;
}

QString AlbumPanel::moduleName()
{
    return "AlbumPanel";
}


QWidget *AlbumPanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;
    tBottomContent->setStyleSheet(this->styleSheet());
    m_slider = new Slider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(3);
    m_slider->setValue(0);
    m_slider->setPageStep(1);
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        if (m_stackWidget->currentWidget() == m_imagesView) {
            int newSize = MIN_ICON_SIZE + multiple * 32;
            m_imagesView->setIconSize(QSize(newSize, newSize));
            dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_IMAGE_ICON_SCALE_KEY,
                                  QVariant(m_slider->value()));
        }
        else {
            m_albumsView->setItemSizeMultiple(multiple);
            dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ALBUM_ICON_SCALE_KEY,
                                  QVariant(m_slider->value()));
        }
    });

    m_countLabel = new QLabel;
    m_countLabel->setObjectName("CountLabel");
    updateAlbumCount();
    updateImagesCount(true);

    ImageButton *ib = new ImageButton;
    ib->setToolTip(tr("Import"));
    ib->setNormalPic(":/images/resources/images/import_normal.png");
    ib->setHoverPic(":/images/resources/images/import_hover.png");
    ib->setPressPic(":/images/resources/images/import_press.png");
    connect(ib, &DImageButton::clicked, this, [=] {
        if (m_stackWidget->currentWidget() == m_imagesView) {
            dApp->importer->showImportDialog(m_imagesView->getCurrentAlbum());
        }
        else {
            dApp->importer->showImportDialog();
        }
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

void AlbumPanel::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        m_stackWidget->setCurrentWidget(m_albumsView);
        // Make sure top toolbar content still show as album content
        // during adding images from timeline
        emit dApp->signalM->gotoPanel(this);
    }
}

void AlbumPanel::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::BackButton) {
        m_stackWidget->setCurrentWidget(m_albumsView);
        // Make sure top toolbar content still show as album content
        // during adding images from timeline
        emit dApp->signalM->gotoPanel(this);
    }
}

void AlbumPanel::initConnection()
{

    connect(dApp->signalM, &SignalManager::createAlbum,
            this, &AlbumPanel::onCreateAlbum);
    connect(dApp->signalM, &SignalManager::importDir,
            this, &AlbumPanel::showImportDirDialog);
    connect(dApp->signalM, &SignalManager::imagesInserted, this, [=] {
        onImageCountChanged(dApp->databaseM->imageCount());
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved, this, [=] {
        onImageCountChanged(dApp->databaseM->imageCount());
    });
    connect(dApp->signalM, &SignalManager::gotoAlbumPanel,
            this, [=] (const QString &album) {
        emit dApp->signalM->gotoPanel(this);
        emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
        emit dApp->signalM->showBottomToolbar();

        if (! album.isEmpty()) {
            onOpenAlbum(album);
        }
    });
}

QWidget *AlbumPanel::toolbarTopLeftContent()
{
    QWidget *tTopleftContent = new QWidget;
    tTopleftContent->setStyleSheet(this->styleSheet());

    QHBoxLayout *layout = new QHBoxLayout(tTopleftContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(ICON_MARGIN);
    if (m_stackWidget->currentWidget() == m_imagesView) {

        ImageButton *returnButton = new ImageButton();

        returnButton->setNormalPic(":/images/resources/images/return_normal.png");
        returnButton->setHoverPic(":/images/resources/images/return_hover.png");
        returnButton->setPressPic(":/images/resources/images/return_press.png");
        connect(returnButton, &ImageButton::clicked, this, [=] {
            m_stackWidget->setCurrentWidget(m_albumsView);
            // Make sure top toolbar content still show as album content
            // during adding images from timeline
            emit dApp->signalM->gotoAlbumPanel();
        });
        returnButton->setToolTip(tr("Back"));
        QLabel *curDirLabel = new QLabel();
        curDirLabel->setObjectName("CurrentDirLabel");
        curDirLabel->setText(m_currentAlbum);
        layout->addWidget(returnButton);
        layout->addWidget(curDirLabel);
    }
    else {
        QLabel *icon = new QLabel;
        // TODO update icon path
        icon->setPixmap(QPixmap(":/images/logo/resources/images/logo/deepin_image_viewer_24.png"));
        layout->addWidget(icon);
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
        emit dApp->signalM->gotoTimelinePanel();
    });
    timelineButton->setToolTip(tr("Timeline"));

    ImageButton *albumButton = new ImageButton();
    albumButton->setNormalPic(":/images/resources/images/album_press.png");
    albumButton->setHoverPic(":/images/resources/images/album_press.png");
    albumButton->setPressPic(":/images/resources/images/album_press.png");
    albumButton->setTooltipVisible(true);

    connect(albumButton, &ImageButton::clicked, this, [=]{
        if (m_stackWidget->currentWidget() == m_imagesView) {
            m_stackWidget->setCurrentWidget(m_albumsView);
        }
    });

    QHBoxLayout *layout = new QHBoxLayout(tTopMiddleContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addSpacing(MARGIN_DIFF);
    layout->addWidget(timelineButton);
    layout->addWidget(albumButton);

    return tTopMiddleContent;
}

QWidget *AlbumPanel::extensionPanelContent()
{
    return NULL;
}

void AlbumPanel::dropEvent(QDropEvent *event)
{
    // "Recent imported" should not handle drop event
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty() || m_currentAlbum == RECENT_IMPORT_ALBUM)
        return;

    QStringList files;
    bool withAlbum = m_stackWidget->currentWidget() == m_imagesView;
    for (QUrl url : urls) {
        const QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            if (!withAlbum) {
                showImportDirDialog(path);
            }
            else {
                dApp->importer->importDir(path, m_currentAlbum);
            }
        }
        else if (utils::image::imageSupportRead(path)) {
            files << path;
        }
    }
    dApp->importer->importFiles(files, withAlbum ? m_currentAlbum : "");
}

void AlbumPanel::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_currentAlbum != RECENT_IMPORT_ALBUM) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void AlbumPanel::initMainStackWidget()
{
    initImagesView();
    initAlbumsView();

    ImportFrame *importFrame = new ImportFrame(this);
    importFrame->setButtonText(tr("Import"));
    importFrame->setTitle(tr("Import or drag image to timeline"));
    connect(importFrame, &ImportFrame::clicked, this, [=] {
        dApp->importer->showImportDialog();
    });

    m_stackWidget = new QStackedWidget;
    m_stackWidget->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->addWidget(importFrame);
    m_stackWidget->addWidget(m_albumsView);
    m_stackWidget->addWidget(m_imagesView);
    //show import frame if no images in database
    m_stackWidget->setCurrentIndex((dApp->databaseM->imageCount() > 0 ||
                                    dApp->databaseM->albumsCount() > 1)? 1 : 0);

    connect(m_stackWidget, &QStackedWidget::currentChanged, this, [=] {
        updateImagesCount(true);
        updateAlbumCount();
        emit dApp->signalM->updateTopToolbarLeftContent(
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
    connect(m_albumsView, &AlbumsView::startSlideShow,
            this, [=] (const QStringList &paths) {
        SignalManager::ViewInfo vinfo;
        vinfo.lastPanel = this;
        vinfo.path = QString();
        vinfo.paths = paths;
        emit dApp->signalM->startSlideShow(vinfo);
    });
}

void AlbumPanel::initImagesView()
{
    m_imagesView = new ImagesView(this);
    connect(m_imagesView, &ImagesView::startSlideShow,
            this, [=] (const QStringList &paths, const QString &path) {
        SignalManager::ViewInfo vinfo;
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths;
        emit dApp->signalM->startSlideShow(vinfo);
    });
    connect(m_imagesView, &ImagesView::viewImage,
            this, [=] (const QString &path, const QStringList &paths, bool f) {
        SignalManager::ViewInfo vinfo;
        vinfo.album = m_imagesView->getCurrentAlbum();
        vinfo.fullScreen = f;
        vinfo.inDatabase = true;
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths;
        emit dApp->signalM->viewImage(vinfo);
    });
    connect(dApp->signalM, &SignalManager::insertIntoAlbum,
            this, &AlbumPanel::onInsertIntoAlbum, Qt::QueuedConnection);
    connect(dApp->signalM, &SignalManager::removedFromAlbum,
            this, [=] (const QString &album, const QStringList &names) {
        if (album == m_imagesView->getCurrentAlbum())
            m_imagesView->removeItems(names);
        updateImagesCount();
    });
    connect(dApp->importer, &Importer::importProgressChanged, this, [=] (double v) {
        if (v == 1) {
            auto infos = dApp->databaseM->getAllImageInfos();
            QStringList names = dApp->databaseM->getImageNamesByAlbum(m_currentAlbum);
            for (auto info : infos) {
                // The albums read from ImageTable is old
                // Read DB one to improve the speed
                if (names.contains(info.name)) {
                    info.albums = QStringList(m_currentAlbum);
                    onInsertIntoAlbum(info);
                }
            }
        }
    });
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

void AlbumPanel::updateImagesCount(bool fromDB)
{
    if (m_countLabel.isNull())
        return;

    // It maybe in import frame
    if (m_stackWidget->currentWidget() == m_albumsView)
        return;

    int count;
    if (fromDB)
        count = dApp->databaseM->getImagesCountByAlbum(m_currentAlbum);
    else
        count = m_imagesView->count();
    QString text = QString::number(count) + " " +
            (count <= 1 ? tr("image") : tr("images"));
    m_countLabel->setText(text);

    if (m_stackWidget->currentWidget() == m_imagesView) {
        m_slider->setValue(dApp->setter->value(SETTINGS_GROUP,
            SETTINGS_IMAGE_ICON_SCALE_KEY, QVariant(0)).toInt());
    }

    m_slider->setFixedWidth(count > 0 ? SLIDER_WIDTH : 0);
}

void AlbumPanel::updateAlbumCount()
{
    if (m_countLabel.isNull())
        return;

    // It maybe in import frame
    if (m_stackWidget->currentWidget() == m_imagesView)
        return;

    const int count = dApp->databaseM->albumsCount();
    QString text = QString::number(count) + " " +
            (count <= 1 ? tr("album") : tr("albums"));
    m_countLabel->setText(text);
    if (m_stackWidget->currentWidget() == m_albumsView) {
        m_slider->setValue(dApp->setter->value(SETTINGS_GROUP,
            SETTINGS_ALBUM_ICON_SCALE_KEY, QVariant(0)).toInt());
    }

    //set width to 1px for layout center
    m_slider->setFixedWidth(count > 0 ? SLIDER_WIDTH : 0);
}

void AlbumPanel::showCreateDialog()
{
    if (! parentWidget()) {
        return;
    }

//TODO: next version change theme to light
//    QSignalBlocker blocker(DThemeManager::instance());
//    Q_UNUSED(blocker);
//    DThemeManager::instance()->setTheme("light");
    CreateAlbumDialog *d = new CreateAlbumDialog(this);
//    DThemeManager::instance()->setTheme("dark");
    connect(d, &CreateAlbumDialog::closed,
            d, &CreateAlbumDialog::deleteLater);
    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
    d->show();
    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
            (parentWidget()->height() - d->height()) / 2 + p.y());

    connect(d, &CreateAlbumDialog::finishedAddAlbum, this, [=]{
        if (m_stackWidget->currentWidget() != m_albumsView &&
                m_stackWidget->currentWidget() != m_imagesView) {
            m_stackWidget->setCurrentWidget(m_albumsView);
        }
    });
}

void AlbumPanel::showImportDirDialog(const QString &dir)
{
    if (! parentWidget() || utils::image::getImagesInfo(dir).isEmpty()) {
        return;
    }

//    QSignalBlocker blocker(DThemeManager::instance());
//    Q_UNUSED(blocker);
//    DThemeManager::instance()->setTheme("light");
    ImportDirDialog *d = new ImportDirDialog(this);
//    DThemeManager::instance()->setTheme("dark");
    connect(d, &ImportDirDialog::albumCreated,
            m_albumsView, &AlbumsView::updateView);
    connect(d, &ImportDirDialog::albumCreated,
            this, &AlbumPanel::updateAlbumCount);
    connect(d, &ImportDirDialog::closed,
            d, &ImportDirDialog::deleteLater);
    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
    d->show();
    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
            (parentWidget()->height() - d->height()) / 2 + p.y());
    d->import(dir);
}

void AlbumPanel::onImageCountChanged(int count)
{
    if (! isVisible())
        return;
     const int albumCounts = dApp->databaseM->albumsCount();
    if (count > 0 && m_stackWidget->currentIndex() == 0) {
        m_stackWidget->setCurrentIndex(1);
    }
    else if (count == 0 && m_stackWidget->currentIndex() == 1
             && albumCounts == 1) {
        m_stackWidget->setCurrentIndex(0);
    }

    updateImagesCount();
}

void AlbumPanel::onInsertIntoAlbum(const DatabaseManager::ImageInfo info)
{
    // No need to update view if importing in others panel, improve performance
    if (m_imagesView->isVisible()
            && info.albums.contains(m_imagesView->getCurrentAlbum())) {
        m_imagesView->insertItem(info);
        updateImagesCount();
    }
}

void AlbumPanel::onOpenAlbum(const QString &album)
{
    m_currentAlbum = album;

    m_stackWidget->setCurrentWidget(m_imagesView);
    const int multiple = dApp->setter->value(SETTINGS_GROUP,
        SETTINGS_IMAGE_ICON_SCALE_KEY, QVariant(0)).toInt();
    int newSize = MIN_ICON_SIZE + multiple * 32;
    m_imagesView->setIconSize(QSize(newSize, newSize));
    m_imagesView->setAlbum(album);

    updateImagesCount(true);
}

void AlbumPanel::onCreateAlbum()
{
    if (this->isVisible() && m_stackWidget->currentWidget() == m_albumsView)
        m_albumsView->createAlbum();
    else
        showCreateDialog();
}

void AlbumPanel::showEvent(QShowEvent *e)
{
    // Make sure BottomContent have been init
    emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
    onImageCountChanged(dApp->databaseM->imageCount());
    ModulePanel::showEvent(e);
}
