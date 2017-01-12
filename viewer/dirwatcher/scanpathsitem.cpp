#include "scanpathsitem.h"
#include "volumemonitor.h"
#include "widgets/imagebutton.h"
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFontMetrics>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QDebug>

namespace {

const int ITEM_WIDTH = 354;
const int ITEM_HEIGHT = 56;
const int ICON_SIZE = 40;
const int LABEL_DEFAULT_WIDTH = 200;
const int MIDDLE_CONTENT_WIDTH = 273;

}  // namespace

// ScanPathsItem
ScanPathsItem::ScanPathsItem(const QString &path)
    : QFrame()
    , m_countTID(0)
    , m_path(path)
{
    setObjectName("PathItem");
    setFixedSize(ITEM_WIDTH, ITEM_HEIGHT);

    // Main layout
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    initLeftIcon();
    initMiddleContent();
    initRemoveIcon();

    QFileSystemWatcher *wc = new QFileSystemWatcher(QStringList(path));
    wc->addPath(path);
    wc->addPath(QFileInfo(path).path());

    connect(wc, &QFileSystemWatcher::directoryChanged,
            this, &ScanPathsItem::updateCount);
    connect(wc, &QFileSystemWatcher::fileChanged,
            this, &ScanPathsItem::updateCount);
    connect(this, &ScanPathsItem::requestUpdateCount,
            this, &ScanPathsItem::updateCount);
    connect(VolumeMonitor::instance(), &VolumeMonitor::deviceAdded,
            this, [=] (const QString &dev) {
        if (path.startsWith(dev)) {
            wc->addPath(path);
            wc->addPath(QFileInfo(path).path());
            updateCount();
        }
    });
}

void ScanPathsItem::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_countTID) {
        killTimer(m_countTID);
        m_countTID = 0;

        CountingThread *ct = new CountingThread(m_path);
        connect(ct, &CountingThread::finished, ct, &CountingThread::deleteLater);
        connect(ct, &CountingThread::ready, this, [=] (const QString &text) {
            if (! dirExist())
                return;
            m_countLabel->setText(text);

            // Length of path and dir label need to be update after count changed
            const int w = m_countLabel->sizeHint().width();
            m_dirLabel->setText(QFontMetrics(m_dirLabel->font()).elidedText(
                                  QFileInfo(m_path).fileName(), Qt::ElideRight,
                                  MIDDLE_CONTENT_WIDTH - 16 - 7 - w));
            m_pathLabel->setText(QFontMetrics(m_pathLabel->font()).elidedText(
                                   m_path, Qt::ElideMiddle,
                                   MIDDLE_CONTENT_WIDTH - 16 -7 - w));
        });
        ct->start();
    }
}

void ScanPathsItem::initLeftIcon()
{
    // Left icon
    QLabel *icon = new QLabel;
    icon->setFixedSize(ICON_SIZE, ICON_SIZE);
    icon->setObjectName("PathItemIcon");
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(icon);
}

void ScanPathsItem::initMiddleContent()
{
    // Middle content
    m_dirLabel = new QLabel;
    m_dirLabel->setObjectName("PathItemDirLabel");
    m_dirLabel->setText(QFontMetrics(m_dirLabel->font()).elidedText(
        QFileInfo(m_path).fileName(), Qt::ElideRight, LABEL_DEFAULT_WIDTH));
    m_pathLabel = new QLabel;
    m_pathLabel->setObjectName("PathItemPathLabel");
    m_pathLabel->setText(QFontMetrics(m_pathLabel->font()).elidedText(
                           m_path, Qt::ElideMiddle, LABEL_DEFAULT_WIDTH));
    QVBoxLayout *dirLayout = new QVBoxLayout;
    dirLayout->setContentsMargins(0, 0, 0, 0);
    dirLayout->setSpacing(0);
    dirLayout->addStretch();
    dirLayout->addWidget(m_dirLabel);
    dirLayout->addWidget(m_pathLabel);
    dirLayout->addStretch();

    m_countLabel = new QLabel;
    m_countLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_countLabel->setObjectName("PathItemCountLabel");

    QFrame *middleContent = new QFrame;
    middleContent->setObjectName("PathItemMiddleContent");
    middleContent->setFixedSize(MIDDLE_CONTENT_WIDTH, ITEM_HEIGHT);
    QHBoxLayout *middleLayout = new QHBoxLayout(middleContent);
    middleLayout->setContentsMargins(0, 0, 7, 0);
    middleLayout->setSpacing(0);
    middleLayout->addLayout(dirLayout);
    middleLayout->addWidget(m_countLabel);

    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(middleContent);
}

void ScanPathsItem::initRemoveIcon()
{
    // Remove icon
    ImageButton *button = new ImageButton;
    button->setFixedSize(1, 1);
    button->setObjectName("PathItemRemoveButton");
    connect(button, &ImageButton::clicked, this, [=] {
        emit remove(m_path);
    });
    connect(this, &ScanPathsItem::showRemoveIconChanged, this, [=] (bool show) {
        if (show) {
            button->setFixedSize(24, 24);
        }
        else {
            button->setFixedSize(1, 1);
        }
    });

    m_mainLayout->addSpacing(7);
    m_mainLayout->addWidget(button, 1, Qt::AlignRight | Qt::AlignVCenter);
    m_mainLayout->addSpacing(1);
}

void ScanPathsItem::updateCount()
{
    style()->unpolish(m_pathLabel);
    if (! dirExist()) {
        m_pathLabel->setProperty("warning", true);
        style()->polish(m_pathLabel);
        m_countLabel->setText(QString("0 ") + tr("Images"));
        if (onMountDevice() && ! mountDeviceExist()) {
            m_pathLabel->setText(tr("The removable device has been plugged out, please plug in again"));
        }
        else {
            // TODO Remove images from DB
            m_pathLabel->setText(tr("This folder has already not exist"));
        }
        return;
    }

    m_pathLabel->setProperty("warning", false);
    style()->polish(m_pathLabel);
    m_countLabel->setText(tr("Calculating..."));
    killTimer(m_countTID);
    m_countTID = startTimer(1000);
}

bool ScanPathsItem::dirExist() const
{
    return QFileInfo(m_path).exists();
}

bool ScanPathsItem::mountDeviceExist() const
{
    QString mountPoint;
    if (m_path.startsWith("/media/")) {
        const int sp = m_path.indexOf("/", 7) + 1;
        const int ep = m_path.indexOf("/", sp) + 1;
        mountPoint = m_path.mid(0, ep);

    }
    else if (m_path.startsWith("/run/media/")) {
        const int sp = m_path.indexOf("/", 11) + 1;
        const int ep = m_path.indexOf("/", sp) + 1;
        mountPoint = m_path.mid(0, ep);
    }

    return QFileInfo(mountPoint).exists();
}

bool ScanPathsItem::onMountDevice() const
{
    return (m_path.startsWith("/media/") || m_path.startsWith("/run/media/"));
}
