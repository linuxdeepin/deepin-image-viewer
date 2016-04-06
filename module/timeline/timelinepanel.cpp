#include "timelinepanel.h"
#include <dimagebutton.h>
#include <dslider.h>
#include <QPushButton>
#include <QFileDialog>
#include <QMimeData>
#include <QLabel>
#include <QDebug>
#include <QUrl>
#include "module/importandexport/importer.h"

using namespace Dtk::Widget;

TimelinePanel::TimelinePanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);

    m_databaseManager = DatabaseManager::instance();//new DatabaseManager("database_counting_connection", this);

    initMainStackWidget();
    initStyleSheet();
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    QWidget *m_tBottomContent = new QWidget;

    QLabel *countLabel = new QLabel;
    countLabel->setObjectName("CountLabel");

    DSlider *slider = new DSlider(Qt::Horizontal);
    slider->setMinimum(1);
    slider->setMaximum(5);
    slider->setValue(1);
    slider->setFixedWidth(120);
    connect(slider, &DSlider::valueChanged, this, [=] (int multiple) {
        qDebug() << "Change the view size to: X" << multiple;
    });

    connect(m_signalManager, &SignalManager::imageCountChanged, this, [=] {
        int count = m_databaseManager->imageCount();
        if (m_databaseManager->imageCount() <= 1) {
            countLabel->setText(tr("%1 Image").arg(count));
        }
        else {
            countLabel->setText(tr("%1 Images").arg(count));
        }

        slider->setFixedWidth(count > 0 ? 120 : 1);
    }, Qt::DirectConnection);

    //init value
    if (m_databaseManager->imageCount() <= 1) {
        countLabel->setText(tr("%1 Image").arg(m_databaseManager->imageCount()));
    }
    else {
        countLabel->setText(tr("%1 Images").arg(m_databaseManager->imageCount()));
    }
    //set width to 1px for layout center
    slider->setFixedWidth(m_databaseManager->imageCount() > 0 ? 120 : 1);

    QHBoxLayout *layout = new QHBoxLayout(m_tBottomContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(countLabel, 1, Qt::AlignHCenter);
    layout->addWidget(slider, 1, Qt::AlignRight);
    layout->addSpacing(16);

    return m_tBottomContent;
}

QWidget *TimelinePanel::toolbarTopLeftContent()
{
    QWidget *m_tTopleftContent = new QWidget;
    QLabel *label = new QLabel;
    label->setPixmap(QPixmap(":/images/icons/resources/images/icons/filter-active.png"));
    QHBoxLayout *layout = new QHBoxLayout(m_tTopleftContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(8);
    layout->addWidget(label, 1, Qt::AlignLeft | Qt::AlignVCenter);
    return m_tTopleftContent;
}

QWidget *TimelinePanel::toolbarTopMiddleContent()
{
    QWidget *m_tTopMiddleContent = new QWidget;

    QLabel *timelineButton = new QLabel();
    timelineButton->setPixmap(QPixmap(":/images/icons/resources/images/icons/timeline-active.png"));

    DImageButton *albumButton = new DImageButton();
    albumButton->setNormalPic(":/images/icons/resources/images/icons/album-normal.png");
    albumButton->setHoverPic(":/images/icons/resources/images/icons/album-hover.png");
    connect(albumButton, &DImageButton::clicked, this, [=] {
        qDebug() << "Change to Album Panel...";
    });

    DImageButton *searchButton = new DImageButton();
    searchButton->setNormalPic(":/images/icons/resources/images/icons/search-normal-24px.png");
    searchButton->setHoverPic(":/images/icons/resources/images/icons/search-hover-24px.png");
    connect(searchButton, &DImageButton::clicked, this, [=] {
        qDebug() << "Change to Search Panel...";
    });

    QHBoxLayout *layout = new QHBoxLayout(m_tTopMiddleContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);
    layout->addStretch(1);
    layout->addWidget(timelineButton);
    layout->addWidget(albumButton);
    layout->addWidget(searchButton);
    layout->addStretch(1);
    return m_tTopMiddleContent;
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

void TimelinePanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void TimelinePanel::initMainStackWidget()
{
    initImagesView();
    initImportFrame();

    m_mainStackWidget = new QStackedWidget;
    m_mainStackWidget->setContentsMargins(0, 0, 0, 0);
    m_mainStackWidget->addWidget(m_importWidget);
    m_mainStackWidget->addWidget(m_imagesView);
    //show import frame if no images in database
    m_mainStackWidget->setCurrentIndex(m_databaseManager->imageCount() > 0 ? 1 : 0);
    connect(m_signalManager, &SignalManager::imageCountChanged, this, [=] {
        m_mainStackWidget->setCurrentIndex(m_databaseManager->imageCount() > 0 ? 1 : 0);
    }, Qt::DirectConnection);

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStackWidget);
}

void TimelinePanel::initImportFrame()
{
    QLabel *il = new QLabel();
    il->setPixmap(QPixmap(":/images/resources/images/timeline_import_backimg.png"));

    QPushButton *ib = new QPushButton(tr("Import"));
    ib->setObjectName("ImportFrameButton");
    connect(ib, &QPushButton::clicked, this, [=] {
        importImages();
    });

    QLabel *tl = new QLabel(tr("You can also drop folder here to import picture"));
    tl->setObjectName("ImportFrameTooltip");

    m_importWidget = new QWidget;
    m_importWidget->setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout(m_importWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(il, 0, Qt::AlignHCenter);
    layout->addSpacing(20);
    layout->addWidget(ib, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(tl, 0, Qt::AlignHCenter);
    layout->addStretch(1);
}

void TimelinePanel::initImagesView()
{
    m_imagesView = new TimelineImageView;
    m_imagesView->setAcceptDrops(true);
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

void TimelinePanel::importImages()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    Importer::instance()->importFromPath(dir);
}
