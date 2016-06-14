#include "albumpanel.h"
#include "createalbumdialog.h"
#include "importdirdialog.h"
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

}   // namespace

AlbumPanel::AlbumPanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);
    initMainStackWidget();
    initStyleSheet();

    connect(m_signalManager, &SignalManager::importDir,
            this, &AlbumPanel::showImportDirDialog);
    connect(m_signalManager, &SignalManager::imageCountChanged,
            this, &AlbumPanel::onImageCountChanged/*, Qt::DirectConnection*/);
}


QWidget *AlbumPanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;
    tBottomContent->setStyleSheet(this->styleSheet());

    m_slider = new Slider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(9);
    m_slider->setValue(0);
    m_slider->setFixedWidth(120);
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        int newSize = MIN_ICON_SIZE + multiple * 32;
        if (m_mainStackWidget->currentWidget() == m_imagesView) {
            m_imagesView->setIconSize(QSize(newSize, newSize));
        }
        else {
            m_albumsView->setItemSize(QSize(newSize, newSize));
        }
    });

    m_countLabel = new QLabel;
    m_countLabel->setAlignment(Qt::AlignLeft);
    m_countLabel->setObjectName("CountLabel");

    updateBottomToolbarContent();

    connect(m_signalManager, &SignalManager::selectImageFromTimeline, this, [=] {
        emit m_signalManager->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit m_signalManager->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    });

    QHBoxLayout *layout = new QHBoxLayout(tBottomContent);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->addStretch(1);
    layout->addWidget(m_countLabel, 1, Qt::AlignCenter);
    layout->addWidget(m_slider, 1, Qt::AlignRight);

    return tBottomContent;
}

QWidget *AlbumPanel::toolbarTopLeftContent()
{
    QWidget *tTopleftContent = new QWidget;
    tTopleftContent->setStyleSheet(this->styleSheet());

    QHBoxLayout *layout = new QHBoxLayout(tTopleftContent);
    layout->setContentsMargins(8, 0, 0, 0);

    if (m_mainStackWidget->currentWidget() == m_imagesView) {
        ImageButton *returnButton = new ImageButton();
        returnButton->setTooltipVisible(true);

        returnButton->setNormalPic(":/images/resources/images/return_normal.png");
        returnButton->setHoverPic(":/images/resources/images/return_hover.png");
        returnButton->setPressPic(":/images/resources/images/return_press.png");
        connect(returnButton, &ImageButton::clicked, this, [=] {
            m_mainStackWidget->setCurrentWidget(m_albumsView);
            // Make sure top toolbar content still show as album content
            // during adding images from timeline
            emit m_signalManager->gotoPanel(this);
        });

        layout->addWidget(returnButton);
        layout->addStretch(1);
    }
    else {
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
        emit m_signalManager->backToMainWindow();
    });
    timelineButton->setToolTip("Timeline");

    QLabel *albumLabel = new QLabel();
    albumLabel->setPixmap(QPixmap(":/images/resources/images/album_active.png"));



    ImageButton *searchButton = new ImageButton();
    searchButton->setNormalPic(":/images/resources/images/search_normal_24px.png");
    searchButton->setHoverPic(":/images/resources/images/search_hover_24px.png");
    searchButton->setPressPic(":/images/resources/images/search_press_24px.png");
    connect(searchButton, &ImageButton::clicked, this, [=] {
        qDebug() << "Change to Search Panel...";
        emit m_signalManager->gotoSearchPanel();
    });
    searchButton->setToolTip("Search");

    QHBoxLayout *layout = new QHBoxLayout(tTopMiddleContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);
    layout->addStretch(1);
    layout->addWidget(timelineButton);
    layout->addWidget(albumLabel);
    layout->addWidget(searchButton);
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
    if (!urls.isEmpty()) {
        for (QUrl url : urls) {
            const QString path = url.toLocalFile();
            if (QFileInfo(path).isDir()) {
                if (m_mainStackWidget->currentWidget() == m_albumsView) {
                    showImportDirDialog(path);
                }
                else {
                    Importer::instance()->importFromPath(
                                path, m_imagesView->getCurrentAlbum());
                }
            }
            else {
                if (utils::image::imageIsSupport(path)) {
                    if (m_mainStackWidget->currentWidget() == m_albumsView) {
                        Importer::instance()->importSingleFile(path);
                    }
                    else {
                        Importer::instance()->importSingleFile(
                                    path, m_imagesView->getCurrentAlbum());
                    }
                }
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

    m_mainStackWidget = new QStackedWidget;
    m_mainStackWidget->setContentsMargins(0, 0, 0, 0);
    m_mainStackWidget->addWidget(new ImportFrame(this));
    m_mainStackWidget->addWidget(m_albumsView);
    m_mainStackWidget->addWidget(m_imagesView);
    //show import frame if no images in database
    m_mainStackWidget->setCurrentIndex(m_dbManager->imageCount() > 0 ? 1 : 0);
    connect(m_mainStackWidget, &QStackedWidget::currentChanged, this, [=] {
        updateBottomToolbarContent();
        emit m_signalManager->updateTopToolbarLeftContent(
                    toolbarTopLeftContent());
    });

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStackWidget);
}

void AlbumPanel::initAlbumsView()
{
    m_albumsView = new AlbumsView(this);
    m_albumsView->updateView();
    connect(m_albumsView, &AlbumsView::openAlbum, this, &AlbumPanel::onOpenAlbum);
}

void AlbumPanel::initImagesView()
{
    m_imagesView = new ImagesView(this);
    connect(m_signalManager, &SignalManager::albumChanged,
            this, [=] (const QString &album) {
        if (m_imagesView->isVisible()
                && album == m_imagesView->getCurrentAlbum()) {
            m_imagesView->updateView();
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

void AlbumPanel::updateBottomToolbarContent()
{
    if (! this->isVisible()) {
        return;
    }

    const int albumCount = m_dbManager->albumsCount();
    const int imagesCount = m_dbManager->getImagesCountByAlbum(m_currentAlbum);
    const bool inAlbumsFrame = m_mainStackWidget->currentIndex() == 1;
    const int count = inAlbumsFrame ? albumCount : imagesCount;

    if (count <= 1) {
        m_countLabel->setText(QString("%1 %2")
                              .arg(count)
                              .arg(inAlbumsFrame ? tr("Album") : tr("Image")));
    }
    else {
        m_countLabel->setText(QString("%1 %2")
                              .arg(count)
                              .arg(inAlbumsFrame ? tr("Albums") : tr("Images")));
    }

    //set width to 1px for layout center
    m_slider->setFixedWidth(count > 0 ? 120 : 1);

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
    if (! parentWidget()) {
        return;
    }
    ImportDirDialog *d = new ImportDirDialog(this, parentWidget());
    connect(d, &ImportDirDialog::closed,
            d, &ImportDirDialog::deleteLater);
    const QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
    d->move((parentWidget()->width() - d->width()) / 2 + p.x(),
            (parentWidget()->height() - d->height()) / 2 + p.y());
    d->import(dir);
}

void AlbumPanel::onImageCountChanged(int count)
{
    updateBottomToolbarContent();
    if (count > 0 && m_mainStackWidget->currentIndex() == 0) {
        m_mainStackWidget->setCurrentIndex(1);
    }
    else if (count == 0) {
        m_mainStackWidget->setCurrentIndex(0);
    }

    if (m_mainStackWidget->currentWidget() == m_imagesView) {
        m_imagesView->updateView();
    }
}

void AlbumPanel::onOpenAlbum(const QString &album)
{
    qDebug() << "Open Album : " << album;
    m_currentAlbum = album;
    m_mainStackWidget->setCurrentIndex(2);
    m_imagesView->setAlbum(album);
}

void AlbumPanel::onCreateAlbum()
{
    if (this->isVisible())
        m_albumsView->createAlbum();
    else
        showCreateDialog();
}

