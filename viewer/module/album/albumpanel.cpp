#include "albumpanel.h"
#include "application.h"
#include "contents/albumbtcontent.h"
#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "widgets/dialogs/albumcreatedialog.h"
#include "widgets/dialogs/dirimportdialog.h"
#include "widgets/imagebutton.h"
#include "widgets/importframe.h"

#include <QDebug>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QPointer>
#include <QPushButton>

DWIDGET_USE_NAMESPACE

namespace {

const int ICON_MARGIN = 13;
const int MARGIN_DIFF = 82;
const QString MY_FAVORITES_ALBUM = "My favorites";
const QString RECENT_IMPORT_ALBUM = "Recent imported";
const QString SETTINGS_GROUP = "ALBUMPANEL";
const QString SETTINGS_ALBUM_ICON_SCALE_KEY = "AlbumIconScale";

}   // namespace

AlbumPanel::AlbumPanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);
    initStyleSheet();
    initMainStackWidget();
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
    m_mContent = new AlbumBTContent;
    m_mContent->setStyleSheet(this->styleSheet());
    m_mContent->setAlbum(m_currentAlbum);
    connect(m_mContent, &AlbumBTContent::itemSizeChanged, this, [=] (int size) {
        m_imagesView->setIconSize(QSize(size, size));
    });
    connect(m_mContent, &AlbumBTContent::multipleChanged, this, [=] (int m) {
        m_albumsView->setItemSizeMultiple(m);
    });

    connect(m_albumsView, &AlbumsView::changeItemSize,
            m_mContent, &AlbumBTContent::changeItemSize);
    connect(m_albumsView, &AlbumsView::albumCreated,
            m_mContent, &AlbumBTContent::updateCount,
            Qt::QueuedConnection);
    connect(m_albumsView, &AlbumsView::albumRemoved,
            m_mContent, &AlbumBTContent::updateCount);

    connect(m_imagesView, &ImagesView::changeItemSize,
            m_mContent, &AlbumBTContent::changeItemSize);

    return m_mContent;
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
    qRegisterMetaType<DBImgInfoList>("DBImgInfoList");
    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, &AlbumPanel::onImageCountChanged);
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &AlbumPanel::onImageCountChanged);
    connect(dApp->signalM, &SignalManager::gotoAlbumPanel,
            this, [=] (const QString &album) {
        emit dApp->signalM->gotoPanel(this);
        emit dApp->signalM->showBottomToolbar();

        if (! m_mContent.isNull()) {
            m_mContent->setInAlbumView(m_stackWidget->currentIndex() == 1);
            m_mContent->updateCount();
            m_mContent->updateSliderDefaultValue();
        }

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
    layout->setSpacing(0);
    layout->addSpacing(ICON_MARGIN);
    if (m_stackWidget->currentWidget() == m_imagesView) {

        ImageButton *returnButton = new ImageButton();
        returnButton->setObjectName("ReturnBtn");

        connect(returnButton, &ImageButton::clicked, this, [=] {
            m_stackWidget->setCurrentWidget(m_albumsView);
            // Make sure top toolbar content still show as album content
            // during adding images from timeline
            emit dApp->signalM->gotoAlbumPanel();
        });
        returnButton->setToolTip(tr("Back"));
        QLabel *curDirLabel = new QLabel();
        curDirLabel->setObjectName("CurrentDirLabel");

        QString an = m_currentAlbum;
        if (m_currentAlbum == MY_FAVORITES_ALBUM) {
            an = tr("My favorites");
        }
        QString dir = curDirLabel->fontMetrics().elidedText(
                    an, Qt::ElideMiddle, tTopleftContent->width()/2);
        curDirLabel->setText(dir);

        layout->addWidget(returnButton);
        layout->addWidget(curDirLabel);
        layout->addStretch();
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
    tTopMiddleContent->setStyleSheet(this->styleSheet());
    ImageButton *timelineButton = new ImageButton();
    timelineButton->setObjectName("TimelineBtn");
    connect(timelineButton, &ImageButton::clicked, this, [=] {
        qDebug() << "Change to Timeline Panel...";
        emit dApp->signalM->gotoTimelinePanel();
    });
    timelineButton->setToolTip(tr("Timeline"));

    ImageButton *albumButton = new ImageButton();
    albumButton->setObjectName("AlbumBtn");
    albumButton->setTooltipVisible(true);

    connect(albumButton, &ImageButton::clicked, this, [=]{
        if (m_stackWidget->currentWidget() == m_imagesView) {
            m_stackWidget->setCurrentWidget(m_albumsView);
        }
    });

    QHBoxLayout *layout = new QHBoxLayout(tTopMiddleContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(timelineButton);
    layout->addWidget(albumButton);
    layout->addStretch();

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
    if (urls.isEmpty())
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
                dApp->importer->appendDir(path, m_currentAlbum);
            }
        }
        else {
            files << path;
        }
    }
    if (! files.isEmpty()) {
        dApp->importer->appendFiles(files, withAlbum ? m_currentAlbum : "");
    }
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

    m_importFrame = new ImportFrame(this);

    m_importFrame->setButtonText(tr("Import"));
    m_importFrame->setTitle(tr("Import or drag image to timeline"));
    connect(m_importFrame, &ImportFrame::clicked, this, [=] {
        dApp->importer->showImportDialog();
    });

    m_stackWidget = new QStackedWidget;
    m_stackWidget->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->addWidget(m_importFrame);
    m_stackWidget->addWidget(m_albumsView);
    m_stackWidget->addWidget(m_imagesView);
    //show import frame if no images in database
    m_stackWidget->setCurrentIndex((dApp->dbM->getImgsCount() > 0 ||
                                    dApp->dbM->getAlbumsCount() > 1) ? 1 : 0);
    connect(m_stackWidget, &QStackedWidget::currentChanged, this, [=] (int i) {
        m_mContent->setInAlbumView(i == 1);
        m_mContent->updateCount();
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
    connect(m_albumsView, &AlbumsView::startSlideShow,
            this, [=] (const QStringList &paths) {
        SignalManager::ViewInfo vinfo;
        vinfo.lastPanel = this;
        vinfo.path = QString();
        vinfo.paths = paths;
        emit dApp->signalM->startSlideShow(vinfo);
    });

    int m = dApp->setter->value(SETTINGS_GROUP,
                                SETTINGS_ALBUM_ICON_SCALE_KEY, 0).toInt();
    m_albumsView->setItemSizeMultiple(m);
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
            this, [=] (const QString &album, const QStringList &paths) {
        if (album == m_imagesView->getCurrentAlbum())
            m_imagesView->removeItems(paths);
        m_mContent->updateCount();
    });
    connect(dApp->importer, &Importer::imported, this, [=] (bool success) {
        if (! success) {
            return;
        }
        auto infos = dApp->dbM->getInfosByAlbum(m_currentAlbum);
        for (auto info : infos) {
            onInsertIntoAlbum(info);
        }
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &AlbumPanel::onThemeChanged);
}

void AlbumPanel::initStyleSheet()
{
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        setStyleSheet(utils::base::getFileContent(":/resources/dark/qss/album.qss"));
    } else {

        setStyleSheet(utils::base::getFileContent(":/resources/light/qss/album.qss"));
    }

}

void AlbumPanel::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        setStyleSheet(utils::base::getFileContent(":/resources/dark/qss/album.qss"));
    } else {
        setStyleSheet(utils::base::getFileContent(":/resources/light/qss/album.qss"));
    }
}

void AlbumPanel::showCreateDialog(QStringList imgpaths)
{
    if (! parentWidget()) {
        return;
    }

    AlbumCreateDialog *d = new AlbumCreateDialog;
    d->show();
//    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
//    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
//            (parentWidget()->height() - d->height()) / 2 + p.y());

    connect(d, &AlbumCreateDialog::albumAdded, this, [=]{
        if (m_stackWidget->currentWidget() != m_albumsView &&
                m_stackWidget->currentWidget() != m_imagesView) {
            m_stackWidget->setCurrentWidget(m_albumsView);
        }

        dApp->dbM->insertIntoAlbum(d->getCreateAlbumName(),  imgpaths.isEmpty()
                                   ? QStringList(" ") : imgpaths);
    });
}

void AlbumPanel::showImportDirDialog(const QString &dir)
{
    if (! parentWidget()) {
        return;
    }

    DirImportDialog *d = new DirImportDialog(dir);
    connect(d, &DirImportDialog::albumCreated,
            m_albumsView, &AlbumsView::updateView);
    connect(d, &DirImportDialog::albumCreated, this, [=] {
        if (! m_mContent.isNull())
            m_mContent->updateCount();
    });
    d->show();
//    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
//    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
//            (parentWidget()->height() - d->height()) / 2 + p.y());
//    d->import(dir);
}

void AlbumPanel::onImageCountChanged()
{
    if (! isVisible())
        return;
    const int count = dApp->dbM->getImgsCount();
    const int albumCounts = dApp->dbM->getAlbumsCount();
    if (count > 0 && m_stackWidget->currentIndex() == 0) {
        m_stackWidget->setCurrentIndex(1);
    }
    else if (count == 0 && m_stackWidget->currentIndex() == 1
             && albumCounts == 1) {
        m_stackWidget->setCurrentIndex(0);
    }
}

void AlbumPanel::onInsertIntoAlbum(const DBImgInfo info)
{
    // No need to update view if importing in others panel, improve performance
    if (m_imagesView->isVisible()
            /*&& dApp->dbM->getPathsByAlbum(m_imagesView->getCurrentAlbum()).contains(info.filePath)*/) {// FIXME
        m_imagesView->insertItem(info);
    }
}

void AlbumPanel::onOpenAlbum(const QString &album)
{
    m_currentAlbum = album;
    m_imagesView->setAlbum(album);

    m_stackWidget->setCurrentWidget(m_imagesView);
    if (! m_mContent.isNull()) {
        m_mContent->setInAlbumView(false);
        m_mContent->updateCount();
        m_mContent->updateSliderDefaultValue();
    }
}

void AlbumPanel::onCreateAlbum(QStringList imagepaths)
{
    if (this->isVisible() && m_stackWidget->currentWidget() == m_albumsView)
        m_albumsView->createAlbum();
    else
        showCreateDialog(imagepaths);
}

void AlbumPanel::showEvent(QShowEvent *e)
{
    // Make sure BottomContent have been init
    emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
    ModulePanel::showEvent(e);
}
