#include "application.h"
#include "scanpathsitem.h"
#include "volumemonitor.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "widgets/imagebutton.h"
#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFontMetrics>
#include <QLabel>
#include <QStringList>
#include <QStyle>
#include <QVBoxLayout>
#include <QDebug>

namespace {

const int ITEM_WIDTH = 354;
const int ITEM_HEIGHT = 56;
const int ICON_SIZE = 40;
const int LABEL_DEFAULT_WIDTH = 200;
const int MIDDLE_CONTENT_WIDTH = 273;

QStringList collectSubDirs(const QString &path)
{
    QStringList dirs;
    QDirIterator dirIterator(path,
                             QDir::Dirs | QDir::NoDotDot | QDir::NoDot,
                             QDirIterator::Subdirectories);
    while(dirIterator.hasNext()) {
        dirIterator.next();
        dirs.append(dirIterator.filePath());
    }
    return dirs;
}

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

    initCountThread();
    initLeftIcon();
    initMiddleContent();
    initRemoveIcon();

    auto subs = collectSubDirs(path);
    subs << path;
    subs << QFileInfo(path).path();
    QFileSystemWatcher *wc = new QFileSystemWatcher(QStringList(path));
    wc->addPaths(subs);

    connect(wc, &QFileSystemWatcher::directoryChanged,
            this, [=] {
        TIMER_SINGLESHOT(1000, {updateCount();}, this)
    });
    connect(this, &ScanPathsItem::requestUpdateCount,
            this, &ScanPathsItem::updateCount);
    connect(VolumeMonitor::instance(), &VolumeMonitor::deviceAdded,
            this, [=] (const QString &dev) {
        if (path.startsWith(dev)) {
            wc->addPaths(subs);
            updateCount();
        }
    });
}

void ScanPathsItem::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_countTID) {
        killTimer(m_countTID);
        m_countTID = 0;

        if (! m_thread->isRunning()) {
            m_thread->start();
        }
    }
}

void ScanPathsItem::initCountThread()
{
    m_thread = new CountingThread(m_path);
    connect(m_thread, &CountingThread::ready, this, [=] (const QString &text) {
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
    button->setToolTip(tr("Cancel sync"));
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
            m_pathLabel->setText(tr("The device has been removed"));
        }
        else {
            // Remove images from DB
            dApp->dbM->removeDir(m_path);
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

CountingThread::CountingThread(const QString &path)
    :QThread()
    ,m_path(path)
{

}

void CountingThread::run()
{
    auto infos = utils::image::getImagesInfo(m_path, true);
    QStringList dirPaths;
    for (auto info : infos) {
        const QString path = info.filePath();
        // Generate thumbnail and storage into cache dir
        if (! utils::image::thumbnailExist(path, utils::image::ThumbLarge)) {
            // Failed thumbnail already exist
            if (utils::image::thumbnailExist(path, utils::image::ThumbFail)) {
                continue;
            }
        }
        dirPaths << path;
    }

    // Remove images which is not exist
    QStringList dbPaths = dApp->dbM->getPathsByDir(m_path);
    QStringList rmPaths;
    for (QString path : dbPaths) {
        if (! dirPaths.contains(path)) {
            rmPaths << path;
        }
    }
    if (! rmPaths.isEmpty()) {
        dApp->dbM->removeImgInfos(rmPaths);
        dbPaths = dApp->dbM->getPathsByDir(m_path);
    }

    // Append the new images
    if (dbPaths.length() != dirPaths.length()) {
        // Reimport
        qDebug() << "New images added, reimport...###" << m_path
                 << dbPaths.length() << infos.length();
        dApp->importer->appendDir(m_path);
    }
    else {
        dirPaths.sort();
        dbPaths.sort();
        if (dirPaths != dbPaths) {
            // Reimport
            qDebug() << "New images added, reimport..." << m_path;
            dApp->importer->appendDir(m_path);
        }
    }

    int count = infos.length();
    emit ready(QString::number(count) + " " + tr("Images"));
}
