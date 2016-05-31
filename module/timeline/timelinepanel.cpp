#include "timelinepanel.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "controller/importer.h"
#include "widgets/importframe.h"
#include "widgets/imagebutton.h"
#include <QPushButton>
#include <QFileDialog>
#include <QMimeData>
#include <QLabel>
#include <QDebug>
#include <QUrl>

const int MIN_ICON_SIZE = 96;
using namespace Dtk::Widget;

TimelinePanel::TimelinePanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);

    m_dbManager = DatabaseManager::instance();

    initMainStackWidget();
    initStyleSheet();
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;
    tBottomContent->setStyleSheet(this->styleSheet());

    QHBoxLayout *layout = new QHBoxLayout(tBottomContent);
    layout->setContentsMargins(14, 0, 14, 0);
    layout->setSpacing(0);

    if (m_targetAlbum.isEmpty()) {
        m_slider = new Dtk::Widget::DSlider(Qt::Horizontal);
        m_slider->setMinimum(0);
        m_slider->setMaximum(9);
        m_slider->setValue(0);
        m_slider->setFixedWidth(120);
        connect(m_slider, &Dtk::Widget::DSlider::valueChanged, this, [=] (int multiple) {
            qDebug() << "Change the view size to: X" << multiple;
            int newSize = MIN_ICON_SIZE + multiple * 32;
            m_imagesView->setIconSize(QSize(newSize, newSize));
        });

        m_countLabel = new QLabel;
        m_countLabel->setObjectName("CountLabel");

        updateBottomToolbarContent();

        connect(m_signalManager, &SignalManager::imageCountChanged,
            this, &TimelinePanel::updateBottomToolbarContent, Qt::DirectConnection);

        layout->addStretch(1);
        layout->addWidget(m_countLabel, 1, Qt::AlignHCenter);
        layout->addWidget(m_slider, 1, Qt::AlignRight);
        layout->addSpacing(16);
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
            m_selectionView->clearSelection();
            emit m_signalManager->updateBottomToolbarContent(toolbarBottomContent());
            emit m_signalManager->gotoAlbumPanel();
        });

        QPushButton *addButton = new QPushButton(tr("Add"));
        addButton->setObjectName("AddToAlbumAdd");
        connect(addButton, &QPushButton::clicked, this, [=] {
            QStringList images = m_selectionView->selectedImages();
            for (QString image : images) {
                // TODO improve performance
                DatabaseManager::ImageInfo info
                        = m_dbManager->getImageInfoByName(image);
                m_dbManager->insertImageIntoAlbum(m_targetAlbum, image,
                    utils::base::timeToString(info.time));
            }

            m_targetAlbum = "";
            m_mainStackWidget->setCurrentWidget(m_imagesView);
            m_selectionView->clearSelection();
            emit m_signalManager->updateBottomToolbarContent(toolbarBottomContent());
            emit m_signalManager->gotoAlbumPanel();
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
    layout->addSpacing(8);
    layout->addWidget(label, 1, Qt::AlignLeft | Qt::AlignVCenter);

    return tTopleftContent;
}

QWidget *TimelinePanel::toolbarTopMiddleContent()
{
    QWidget *tTopMiddleContent = new QWidget;

    QLabel *timelineButton = new QLabel();

    timelineButton->setPixmap(QPixmap(":/images/resources/images/timeline_active.png"));

    ImageButton *albumButton = new ImageButton();
    albumButton->setNormalPic(":/images/resources/images/album_normal.png");
    albumButton->setHoverPic(":/images/resources/images/album_hover.png");
    connect(albumButton, &ImageButton::clicked, this, [=] {
        qDebug() << "Change to Album Panel...";
        emit m_signalManager->gotoAlbumPanel();
    });
    albumButton->setToolTip("Album");
    ImageButton *searchButton = new ImageButton();
    searchButton->setNormalPic(":/images/resources/images/search_normal_24px.png");
    searchButton->setHoverPic(":/images/resources/images/search_hover_24px.png");
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
    layout->addWidget(albumButton);
    layout->addWidget(searchButton);
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
                emit m_signalManager->importDir(path);
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

    connect(m_signalManager, &SignalManager::imageCountChanged, this, [=] {
        m_mainStackWidget->setCurrentIndex(m_dbManager->imageCount() > 0 ? 1 : 0);
    }, Qt::DirectConnection);
    connect(m_signalManager, &SignalManager::selectImageFromTimeline,
            this, [=] (const QString &targetAlbum) {
        m_targetAlbum = targetAlbum;
        m_mainStackWidget->setCurrentWidget(m_selectionView);
        emit m_signalManager->gotoPanel(this);
        emit m_signalManager->updateBottomToolbarContent(toolbarBottomContent(), true);
    });

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStackWidget);
}

void TimelinePanel::initImagesView()
{
    m_imagesView = new TimelineImageView;
    m_imagesView->setAcceptDrops(true);

    // To make MainWindow load faster
    QTimer::singleShot(100, m_imagesView, SLOT(insertReadyFrames()));
//    QMetaObject::invokeMethod(m_imagesView, "insertReadyFrames",
//                              Qt::QueuedConnection);
}

void TimelinePanel::initSelectionView()
{
    m_selectionView = new TimelineImageView(true);
    m_selectionView->setAcceptDrops(false);
    QTimer::singleShot(100, m_imagesView, SLOT(insertReadyFrames()));
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

void TimelinePanel::updateBottomToolbarContent()
{
    if (! this->isVisible()) {
        return;
    }

    int count = m_dbManager->imageCount();
    if (count <= 1) {
        m_countLabel->setText(tr("%1 Image").arg(count));
    }
    else {
        m_countLabel->setText(tr("%1 Images").arg(count));
    }

    m_slider->setFixedWidth(count > 0 ? 120 : 1);
}
