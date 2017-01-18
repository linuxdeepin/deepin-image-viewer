#include "scanpathsdialog.h"
#include "scanpathsitem.h"
#include "application.h"
#include <ddialog.h>
#include <dtitlebar.h>
#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE

namespace {

const int DIALOG_WIDTH = 382;
const int DIALOG_HEIGHT = 390;
const int MESSAGE_DURATION = 2000;

const QString SCANPATHS_GROUP = "SCANPATHSGROUP";
const QString SCANPATHS_KEY = "SCANPATHSKEY";

}  // namespace

ScanPathsDialog::ScanPathsDialog(QWidget *parent)
    : DMainWindow(parent)
    ,m_messageTID(0)
{
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);
    if (titleBar()) titleBar()->setFixedHeight(0);
    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT);
    setStyleSheet(utils::base::getFileContent(
                      ":/dirwatcher/qss/resources/qss/scanpathsdialog.qss"));

    QWidget *w = new QWidget(this);
    setCentralWidget(w);
    m_mainLayout = new QVBoxLayout(w);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    initTitle();
    initPathsArea();
    initMessageLabel();
    initAddButton();
    initSinglaFileWatcher();
}

ScanPathsDialog *ScanPathsDialog::m_dialog = NULL;
ScanPathsDialog *ScanPathsDialog::instance()
{
    if (! m_dialog) {
        m_dialog = new ScanPathsDialog();
    }

    return m_dialog;
}

void ScanPathsDialog::show()
{
    DMainWindow::show();

    QRect gr;
    QPoint pos = QCursor::pos();
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            gr = screen->geometry();
            break;
        }
    }
    QRect qr = geometry();
    qr.moveCenter(gr.center());
    move(qr.topLeft());

//    emit requestUpdateCount();
}

void ScanPathsDialog::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_messageTID) {
        m_messageLabel->setText("");
        killTimer(m_messageTID);
        m_messageTID = 0;
    }
    else {
        DMainWindow::timerEvent(e);
    }
}

void ScanPathsDialog::showSelectDialog()
{
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setWindowTitle(tr("Select Directory"));
    dialog->setDirectory(QStandardPaths::standardLocations(
                             QStandardPaths::PicturesLocation).first());
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOptions(
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dialog->exec()) {
        auto files = dialog->selectedFiles();
        if (! files.isEmpty()) {
            addPath(files.first());
        }

        dialog->deleteLater();
    } else {
        dialog->deleteLater();
    }
}

void ScanPathsDialog::addPath(const QString &path, bool check)
{
    if (check && (path.isEmpty() || ! isLegalPath(path))) {
        // If path can't be select
        showMessage(tr("Sync of this directory is not allowed"));
    }
    else if (check && isContainByScanPaths(path)) {
        // If path is already in scan paths list
//        showMessage(tr("The path is already in scan paths list"));
    }
    else if (check && isSubPathOfScanPaths(path)) {
        // If path is contain by others
//        showMessage(tr("The path is contain by scan paths list"));
    }
    else {
        if (check) {
            addToScanPaths(path);
            m_contentStack->setCurrentIndex(1);
        }

        ScanPathsItem *item = new ScanPathsItem(path);
        emit item->requestUpdateCount();
        connect(this, &ScanPathsDialog::requestUpdateCount,
                item, &ScanPathsItem::requestUpdateCount);
        connect(item, &ScanPathsItem::remove, this, [=] (const QString &path) {
            removePath(path);
            m_pathsLayout->removeWidget(item);
            item->deleteLater();
        });
        m_pathsLayout->addWidget(item);
    }
}

void ScanPathsDialog::removePath(const QString &path)
{
    qDebug() << "Import Thread has been stoped, removing data from DB...";

    // Remove data from DB
    dApp->dbM->removeDir(path);

    // Remove from config-file
    removeFromScanPaths(path);
    if (scanpaths().isEmpty()) {
        m_contentStack->setCurrentIndex(0);
    }

    qDebug() << "Data has been removed from DB.";
}

void ScanPathsDialog::initAddButton()
{
    QPushButton * button = new QPushButton(tr("Add folder"));
    button->setFixedSize(310, 39);
    button->setObjectName("AddButton");
    m_mainLayout->addSpacing(7);
    m_mainLayout->addWidget(button, 1, Qt::AlignCenter | Qt::AlignBottom);
    m_mainLayout->addSpacing(27);

    connect(button, SIGNAL(clicked(bool)), this, SLOT(showSelectDialog()));
}

void ScanPathsDialog::initSinglaFileWatcher()
{
    QFileSystemWatcher *wc = new QFileSystemWatcher();

    QStringList rmPaths;
    QStringList dirs;
    QStringList dbPaths = dApp->dbM->getPathsByDir(QString());
    for (auto dbp : dbPaths) {
        QFileInfo info(dbp);
        if (! info.exists()) {
            rmPaths << dbp;
        }
        else {
            QString dir = info.path();
            if (dirs.indexOf(dir) == -1) {
                dirs << dir;
            }
        }
    }

    // Remove the images which is not exist
    dApp->dbM->removeImgInfos(rmPaths);

    wc->addPaths(dirs);

    connect(wc, &QFileSystemWatcher::directoryChanged,
            this, [=] (const QString dir){
        QStringList rmPaths;
        QStringList dbPaths = dApp->dbM->getPathsByDir(QString());
        for (auto dbp : dbPaths) {
            QFileInfo info(dbp);
            if (info.path() == dir && ! info.exists()) {
                rmPaths << dbp;
            }
        }
        // Remove the images which is not exist
        dApp->dbM->removeImgInfos(rmPaths);
    });
    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, [=] (const DBImgInfoList &infos) {
        for (auto info : infos) {
            const QString emptyHash = utils::base::hash(QString());
            if (info.dirHash == emptyHash) {
                QFileInfo fi(info.filePath);
                wc->addPath(fi.path());
            }
        }
    });
}

void ScanPathsDialog::initTitle()
{
    QLabel *title = new QLabel(tr("Auto scan directory manage"));
    title->setAlignment(Qt::AlignCenter);
    title->setObjectName("TitleLabel");

    ImageButton* cb = new ImageButton(this);
    cb->setObjectName("CloseButton");
    cb->setTooltipVisible(true);
    cb->setFixedSize(24, 24);
    connect(cb, &ImageButton::clicked, this, &ScanPathsDialog::hide);

    QWidget *w = new QWidget;
    w->setFixedHeight(27);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(24 + 5, 0, 5, 0);
    layout->setSpacing(0);

    layout->addWidget(title);
    layout->addWidget(cb);
    m_mainLayout->addSpacing(2);
    m_mainLayout->addWidget(w);
}

void ScanPathsDialog::initPathsArea()
{
    // Empty frame
    QLabel *el = new QLabel(tr("The folder list is empty"));
    el->setAlignment(Qt::AlignCenter);
    el->setObjectName("EmptyFrame");

    // ScrollArea
    QScrollArea *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setObjectName("PathsScrollArea");

    QWidget *areaContent = new QWidget;
    areaContent->setObjectName("PathsContent");
    m_pathsLayout = new QVBoxLayout(areaContent);
    m_pathsLayout->setAlignment(Qt::AlignTop);
    m_pathsLayout->setContentsMargins(0, 0, 0, 0);
    m_pathsLayout->setSpacing(0);
    area->setWidget(areaContent);

    m_contentStack = new QStackedWidget;
    m_contentStack->setContentsMargins(0, 0, 0, 0);
    m_contentStack->setFixedSize(354, 250);
    m_contentStack->addWidget(el);
    m_contentStack->addWidget(area);
    if (scanpaths().length() > 0) {
        m_contentStack->setCurrentIndex(1);
        auto paths = scanpaths();
        for (auto p : paths) {
            addPath(p, false);
        }
    }

    m_mainLayout->addSpacing(14);
    m_mainLayout->addWidget(m_contentStack, 1, Qt::AlignCenter);
}

void ScanPathsDialog::initMessageLabel()
{
    m_messageLabel = new QLabel;
    m_messageLabel->setObjectName("MessageLabel");
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_messageLabel);
}

void ScanPathsDialog::showMessage(const QString &message)
{
    killTimer(m_messageTID);
    m_messageTID = startTimer(MESSAGE_DURATION);

    m_messageLabel->setText(message);
}

bool ScanPathsDialog::isLegalPath(const QString &path) const
{
#ifdef Q_OS_LINUX
    QStringList legalPrefixs;
    legalPrefixs << QDir::homePath() + "/" << "/media/" << "/run/media/";
    for (QString prefix : legalPrefixs) {
        if (path.startsWith(prefix)) {
            return true;
        }
    }
    return false;
#elif
    return true;
#endif
}

bool ScanPathsDialog::isContainByScanPaths(const QString &path) const
{
    auto paths = scanpaths();
    for (auto p : paths) {
        if (p == path) {
            return true;
        }
    }

    return false;
}

bool ScanPathsDialog::isSubPathOfScanPaths(const QString &path) const
{
    auto paths = scanpaths();
    for (auto p : paths) {
        if (path.startsWith(p)) {
            return true;
        }
    }

    return false;
}

QStringList ScanPathsDialog::scanpaths() const
{
    QStringList paths = dApp->setter->value(SCANPATHS_GROUP, SCANPATHS_KEY)
            .toString().split(",");
    paths.removeAll("");
    return paths;
}

void ScanPathsDialog::addToScanPaths(const QString &path)
{
    auto paths = scanpaths();
    if (paths.contains(path))
        return;
    paths.append(path);
    QString v;
    for (auto p : paths) {
        v += p + ",";
    }
    v.remove(v.length() - 1, 1);
    dApp->setter->setValue(SCANPATHS_GROUP, SCANPATHS_KEY, v);
}

void ScanPathsDialog::removeFromScanPaths(const QString &path)
{
    auto paths = scanpaths();
    paths.removeAll(path);
    QString v;
    for (auto p : paths) {
        v += p + ",";
    }
    v.remove(v.length() - 1, 1);
    dApp->setter->setValue(SCANPATHS_GROUP, SCANPATHS_KEY, v);
}
