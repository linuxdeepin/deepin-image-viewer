#include "scanpathsdialog.h"
#include "application.h"
#include <ddialog.h>
#include <dtitlebar.h>
#include "controller/configsetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "../imagebutton.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QThread>
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

// CountingThread
class CountingThread : public QThread {
    Q_OBJECT
public:
    CountingThread(const QString &path)
        :QThread()
        ,m_path(path) {}

    void run() Q_DECL_OVERRIDE {
        int count = utils::image::getImagesInfo(m_path, true).length();
        emit ready(QString::number(count) + " " + tr("Images"));
    }

signals:
    void ready(const QString &text);

private:
    QString m_path;
};

// PathItem
class PathItem : public QFrame {
    Q_OBJECT
public:
    PathItem(const QString &path);

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE {
        QFrame::enterEvent(e);
        emit showRemoveIconChanged(true);
    }
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE {
        QFrame::leaveEvent(e);
        emit showRemoveIconChanged(false);
    }

signals:
    void remove(QString path);
    void requestUpdateCount();
    void showRemoveIconChanged(bool show);
};

ScanPathsDialog::ScanPathsDialog(QWidget *parent)
    : DMainWindow(parent)
    ,m_messageTID(0)
{
    setWindowModality(Qt::ApplicationModal);
    if (titleBar()) titleBar()->setFixedHeight(0);
    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT);
    setStyleSheet(utils::base::getFileContent(
                      ":/dialogs/qss/resources/qss/scanpathsdialog.qss"));

    QWidget *w = new QWidget(this);
    setCentralWidget(w);
    m_mainLayout = new QVBoxLayout(w);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    initTitle();
    initPathsArea();
    initMessageLabel();
    initAddButton();

//    // Add Pictures as default
//    const QStringList picturesPaths =
//            QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
//    if (! picturesPaths.isEmpty()) {
//        addPath(picturesPaths.first());
//    }
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

    emit requestUpdateCount();
}

void ScanPathsDialog::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_messageTID) {
        m_messageLabel->setText("");
        killTimer(m_messageTID);
    }
    else {
        DMainWindow::timerEvent(e);
    }
}

void ScanPathsDialog::showSelectDialog()
{
    QFileDialog *dialog = new QFileDialog;
    dialog->setWindowTitle(tr("Select Directory"));
    dialog->setDirectory(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dialog->exec()) {
        auto files = dialog->selectedFiles();
        if (! files.isEmpty()) {
            addPath(files.first());
        }
    }
}

void ScanPathsDialog::addPath(const QString &path, bool check)
{
    if (check && (path.isEmpty() || ! isLegalPath(path))) {
        // If path can't be select
        showMessage(tr("This Directory can not be select"));
    }
    else if (check && isContainByScanPaths(path)) {
        // If path is already in scan paths list
        showMessage(tr("The path is already in scan paths list"));
    }
    else if (check && isSubPathOfScanPaths(path)) {
        // If path is contain by others
        showMessage(tr("The path is contain by scan paths list"));
    }
    else {
        if (check) {
            addToScanPaths(path);
            m_contentStack->setCurrentIndex(1);
        }

        PathItem *item = new PathItem(path);
        emit item->requestUpdateCount();
        connect(this, &ScanPathsDialog::requestUpdateCount,
                item, &PathItem::requestUpdateCount);
        connect(item, &PathItem::remove, this, [=] (const QString &path) {
            removePath(path);
            m_pathsLayout->removeWidget(item);
            item->deleteLater();
        });
        m_pathsLayout->addWidget(item);
    }
}

void ScanPathsDialog::removePath(const QString &path)
{
    removeFromScanPaths(path);
    if (scanpaths().isEmpty()) {
        m_contentStack->setCurrentIndex(0);
    }
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


#include "scanpathsdialog.moc"
PathItem::PathItem(const QString &path)
    : QFrame()
{
    setObjectName("PathItem");
    setFixedSize(354, 56);

    // Left icon
    QLabel *icon = new QLabel;
    icon->setFixedSize(40, 40);
    icon->setObjectName("PathItemIcon");

    // Middle content
    QLabel *dirLabel = new QLabel;
    dirLabel->setObjectName("PathItemDirLabel");
    dirLabel->setText(QFontMetrics(dirLabel->font()).elidedText(
                          QFileInfo(path).fileName(), Qt::ElideRight, 200));
    QLabel *pathLabel = new QLabel;
    pathLabel->setObjectName("PathItemPathLabel");
    pathLabel->setText(QFontMetrics(pathLabel->font()).elidedText(
                           path, Qt::ElideMiddle, 200));
    QVBoxLayout *dirLayout = new QVBoxLayout;
    dirLayout->setContentsMargins(0, 0, 0, 0);
    dirLayout->setSpacing(0);
    dirLayout->addStretch();
    dirLayout->addWidget(dirLabel);
    dirLayout->addWidget(pathLabel);
    dirLayout->addStretch();

    QLabel *countLabel = new QLabel;
    countLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    countLabel->setObjectName("PathItemCountLabel");
    connect(this, &PathItem::requestUpdateCount, this, [=] {
        if (countLabel->text().isEmpty()) {
            countLabel->setText(tr("Calculating..."));
        }
        CountingThread *ct = new CountingThread(path);
        connect(ct, &CountingThread::ready, this, [=] (const QString &text) {
            countLabel->setText(text);
            // Length of path and dir label need to be update after count changed
            const int w = countLabel->sizeHint().width();
            dirLabel->setText(QFontMetrics(dirLabel->font()).elidedText(
                                  QFileInfo(path).fileName(), Qt::ElideRight, 273 - 16 - 7 - w));
            pathLabel->setText(QFontMetrics(pathLabel->font()).elidedText(
                                   path, Qt::ElideMiddle, 273 - 16 -7 - w));
        });
        connect(ct, &CountingThread::finished, ct, &CountingThread::deleteLater);
        ct->start();
    });

    QFrame *middleContent = new QFrame;
    middleContent->setObjectName("PathItemMiddleContent");
    middleContent->setFixedSize(273, 56);
    QHBoxLayout *middleLayout = new QHBoxLayout(middleContent);
    middleLayout->setContentsMargins(0, 0, 7, 0);
    middleLayout->setSpacing(0);
    middleLayout->addLayout(dirLayout);
    middleLayout->addWidget(countLabel);

    // Remove icon
    ImageButton *button = new ImageButton;
    button->setFixedSize(1, 1);
    button->setObjectName("PathItemRemoveButton");
    connect(button, &ImageButton::clicked, this, [=] {
        emit remove(path);
    });
    connect(this, &PathItem::showRemoveIconChanged, this, [=] (bool show) {
        if (show) {
            button->setFixedSize(24, 24);
        }
        else {
            button->setFixedSize(1, 1);
        }
    });

    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(icon);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(middleContent);
    mainLayout->addSpacing(7);
    mainLayout->addWidget(button, 1, Qt::AlignRight | Qt::AlignVCenter);
    mainLayout->addSpacing(1);
}
