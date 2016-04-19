#include "timelinepanel.h"
#include "module/importandexport/importer.h"
#include "widgets/importframe.h"
#include <dimagebutton.h>
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

    m_databaseManager = DatabaseManager::instance();

    initMainStackWidget();
    initStyleSheet();
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    QWidget *tBottomContent = new QWidget;

    m_slider = new DSlider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(9);
    m_slider->setValue(0);
    m_slider->setFixedWidth(120);
    connect(m_slider, &DSlider::valueChanged, this, [=] (int multiple) {
        qDebug() << "Change the view size to: X" << multiple;
        int newSize = MIN_ICON_SIZE + multiple * 32;
        m_imagesView->setIconSize(QSize(newSize, newSize));
    });

    m_countLabel = new QLabel;
    m_countLabel->setObjectName("CountLabel");

    updateBottomToolbarContent();

    connect(m_signalManager, &SignalManager::imageCountChanged,
        this, &TimelinePanel::updateBottomToolbarContent, Qt::DirectConnection);

    QHBoxLayout *layout = new QHBoxLayout(tBottomContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_countLabel, 1, Qt::AlignHCenter);
    layout->addWidget(m_slider, 1, Qt::AlignRight);
    layout->addSpacing(16);

    return tBottomContent;
}

QWidget *TimelinePanel::toolbarTopLeftContent()
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

QWidget *TimelinePanel::toolbarTopMiddleContent()
{
    QWidget *tTopMiddleContent = new QWidget;

    QLabel *timelineButton = new QLabel();
    timelineButton->setPixmap(QPixmap(":/images/icons/resources/images/icons/timeline-active.png"));

    DImageButton *albumButton = new DImageButton();
    albumButton->setNormalPic(":/images/icons/resources/images/icons/album-normal.png");
    albumButton->setHoverPic(":/images/icons/resources/images/icons/album-hover.png");
    connect(albumButton, &DImageButton::clicked, this, [=] {
        qDebug() << "Change to Album Panel...";
        emit needGotoAlbumPanel();
    });

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

    m_mainStackWidget = new QStackedWidget;
    m_mainStackWidget->setContentsMargins(0, 0, 0, 0);
    m_mainStackWidget->addWidget(new ImportFrame(this));
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

void TimelinePanel::updateBottomToolbarContent()
{
    if (! this->isVisible()) {
        return;
    }

    int count = m_databaseManager->imageCount();
    if (count <= 1) {
        m_countLabel->setText(tr("%1 Image").arg(count));
    }
    else {
        m_countLabel->setText(tr("%1 Images").arg(count));
    }

    m_slider->setFixedWidth(count > 0 ? 120 : 1);
}
