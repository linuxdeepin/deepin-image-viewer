#include "albumpanel.h"
#include "controller/databasemanager.h"
#include "module/importandexport/importer.h"
#include "widgets/importframe.h"
#include <dimagebutton.h>
#include <QFileInfo>
#include <QDropEvent>
#include <QPushButton>
#include <QPointer>
#include <QDebug>

namespace {

const int MIN_ICON_SIZE = 96;

}   // namespace

using namespace Dtk::Widget;

AlbumPanel::AlbumPanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);
    initMainStackWidget();
    initStyleSheet();
}


QWidget *AlbumPanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;

    m_slider = new DSlider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(9);
    m_slider->setValue(0);
    m_slider->setFixedWidth(120);
    connect(m_slider, &DSlider::valueChanged, this, [=] (int multiple) {
        int newSize = MIN_ICON_SIZE + multiple * 32;
        //            m_imagesView->setIconSize(QSize(newSize, newSize));
    });

    m_countLabel = new QLabel;
    m_countLabel->setObjectName("CountLabel");

    updateBottomToolbarContent();

    connect(m_signalManager, &SignalManager::imageCountChanged,
            this, &AlbumPanel::updateBottomToolbarContent, Qt::DirectConnection);

    QHBoxLayout *layout = new QHBoxLayout(tBottomContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_countLabel, 1, Qt::AlignHCenter);
    layout->addWidget(m_slider, 1, Qt::AlignRight);
    layout->addSpacing(16);

    return tBottomContent;
}

QWidget *AlbumPanel::toolbarTopLeftContent()
{
    QWidget *tTopleftContent = new QWidget;
    QLabel *label = new QLabel;
    label->setPixmap(QPixmap(":/images/icons/resources/images/icons/filter-active.png"));
    QHBoxLayout *layout = new QHBoxLayout(tTopleftContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(8);
    layout->addWidget(label, 1, Qt::AlignLeft | Qt::AlignVCenter);

    return tTopleftContent;
}

QWidget *AlbumPanel::toolbarTopMiddleContent()
{
    QWidget *tTopMiddleContent = new QWidget;

    DImageButton *timelineButton = new DImageButton();
    timelineButton->setNormalPic(":/images/icons/resources/images/icons/timeline-normal.png");
    timelineButton->setHoverPic(":/images/icons/resources/images/icons/timeline-hover.png");
    connect(timelineButton, &DImageButton::clicked, this, [=] {
        qDebug() << "Change to Timeline Panel...";
        emit needGotoTimelinePanel();
    });

    QLabel *albumLabel = new QLabel();
    albumLabel->setPixmap(QPixmap(":/images/icons/resources/images/icons/album-active.png"));

    DImageButton *searchButton = new DImageButton();
    searchButton->setNormalPic(":/images/icons/resources/images/icons/search-normal-24px.png");
    searchButton->setHoverPic(":/images/icons/resources/images/icons/search-hover-24px.png");
    connect(searchButton, &DImageButton::clicked, this, [=] {
        qDebug() << "Change to Search Panel...";
        emit needGotoSearchPanel();
    });

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
            QFileInfo info(url.toLocalFile());
            if (info.isDir()) {
                Importer::instance()->importFromPath(url.toLocalFile());
            }
            else {
                if (DatabaseManager::instance()->supportImageType().indexOf(info.suffix()) != 0) {
                    Importer::instance()->importSingleFile(url.toLocalFile());
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
    m_mainStackWidget->setCurrentIndex(m_databaseManager->imageCount() > 0 ? 1 : 0);
    connect(m_mainStackWidget, &QStackedWidget::currentChanged, this, [=] {
        updateBottomToolbarContent();
    });
    connect(m_signalManager, &SignalManager::imageCountChanged, this, [=] {
        m_mainStackWidget->setCurrentIndex(m_databaseManager->imageCount() > 0 ? 1 : 0);
    }, Qt::DirectConnection);

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStackWidget);
}

void AlbumPanel::initAlbumsView()
{
    m_albumsView = new AlbumsView(this);
}

void AlbumPanel::initImagesView()
{
    m_imagesView = new ImagesView(this);
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

    const int albumCount = m_databaseManager->albumsCount();
    const int imagesCount = m_databaseManager->imageCount();
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

