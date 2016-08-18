#include "toptoolbar.h"
#include "application.h"
#include "controller/importer.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "frame/mainwindow.h"
#include "frame/aboutwindow.h"
#include "widgets/progresswidgetstips.h"
#include "utils/shortcut.h"
#include <dcircleprogress.h>
#include <dwindowminbutton.h>
#include <dwindowclosebutton.h>
#include <dwindowoptionbutton.h>
#include <dwindowrestorebutton.h>
#include <darrowrectangle.h>
#include <QDebug>
#include <QGradient>
#include <QPainter>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QShortcut>

//#include <dthememanager.h>
using namespace Dtk::Widget;

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int ICON_MARGIN = 6;

}  // namespace

TopToolbar::TopToolbar(QWidget *parent, QWidget *source)
    :BlureFrame(parent, source)
{
    QLinearGradient linearGrad;
    linearGrad.setColorAt(0, QColor(15, 15, 15, 178));
    linearGrad.setColorAt(1, QColor(15, 15, 15, 204));

    setCoverBrush(QBrush(linearGrad));

//    initAboutWindow();
    m_about = new AboutWindow(parent, source);
    m_about->hide();

    initWidgets();
    initMenu();

    QShortcut* viewScut = new QShortcut(QKeySequence("Ctrl+Shift+/"),
                                        this);
    connect(viewScut, &QShortcut::activated, this, &TopToolbar::showShortCutView);
    qApp->installEventFilter(this);
    parent->installEventFilter(this);
    connect(this, SIGNAL(moving()),
            parentWidget()->parentWidget(), SLOT(startMoving()));
}

void TopToolbar::setLeftContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_leftLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    m_leftLayout->addWidget(content);
}

void TopToolbar::setMiddleContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_middleLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_middleLayout->addWidget(content);
}

bool TopToolbar::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Resize) {
        if (! window()->isFullScreen()) {
            m_maxb->setMaximized(window()->isMaximized());
        }
    }
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke && ke->key() == Qt::Key_F1) {
            showManual();
        }
    }
    return false;
}

void TopToolbar::resizeEvent(QResizeEvent *e)
{
    m_leftContent->setFixedWidth(e->size().width() / 3);
    m_middleContent->setFixedWidth(e->size().width() / 3);
    m_rightContent->setFixedWidth(e->size().width() / 3);
}

void TopToolbar::mousePressEvent(QMouseEvent *e)
{
    BlureFrame::mousePressEvent(e);
    m_pressBtn = e->button();
}

void TopToolbar::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (m_pressBtn == Qt::LeftButton)
        emit moving();
}

void TopToolbar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (window()->isMaximized())
            window()->showNormal();
        else if (! window()->isFullScreen())  // It would be normal state
            window()->showMaximized();
    }
}

void TopToolbar::paintEvent(QPaintEvent *e)
{
    BlureFrame::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw inside top border
    const QColor tc(255, 255, 255, 20);
    const int borderHeight = 1;
    QPainterPath tPath;
    tPath.moveTo(QPointF(x(), y() + borderHeight - 0.5));
    tPath.lineTo(QPointF(x() + width(), y() + borderHeight - 0.5));

    QPen tPen(tc, borderHeight);
    QLinearGradient linearGrad;
    linearGrad.setStart(x(), y() + borderHeight);
    linearGrad.setFinalStop(x() + width(), y() + borderHeight);
    linearGrad.setColorAt(0, Qt::transparent);
    linearGrad.setColorAt(0.005, tc);
    linearGrad.setColorAt(0.995, tc);
    linearGrad.setColorAt(1, Qt::transparent);
    tPen.setBrush(QBrush(linearGrad));
    p.setPen(tPen);
    p.drawPath(tPath);

    // Draw inside bottom border
    QPainterPath bPath;
    bPath.moveTo(x(), y() + height() - borderHeight);
    bPath.lineTo(x() + width(), y() + height() - borderHeight);
    QPen bPen(QColor(0, 0, 0, 25), borderHeight);
    p.setPen(bPen);
    p.drawPath(bPath);
}

//void TopToolbar::initAboutWindow() {
//    QString icon(":/images/resources/images/deepin_image_viewer.png");
//    QSignalBlocker blocker(DThemeManager::instance());
//    Q_UNUSED(blocker);
//    DThemeManager::instance()->setTheme("light");

    //TODO: if without '\n' the text will be cut.
//    QString description = QString(tr("Deepin Image Viewer is a fashion & smooth image manager, "
//     "It is featured with image management, "
//     "image viewing and basic image editing."));

//    m_about = new DAboutDialog(icon, icon, qApp->applicationDisplayName(),
//                               tr("Version:") + qApp->applicationVersion(),
//                               description, this);


//    DThemeManager::instance()->setTheme("dark");
//    m_about->setTitle("");
//    m_about->hide();
//}

void TopToolbar::initWidgets()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    DCircleProgress *importProgress = new DCircleProgress;
    importProgress->setValue(0);
    importProgress->setFixedSize(21, 21);
    importProgress->setVisible(false);
    //importProgress's tooltip begin to init;
    DArrowRectangle *importProgressWidget =
            new DArrowRectangle(DArrowRectangle::ArrowTop);
    importProgressWidget->setArrowX(180);
    importProgressWidget->setArrowWidth(15);
    importProgressWidget->setArrowHeight(9);

    ProgressWidgetsTips* progressWidgetTips = new ProgressWidgetsTips;
    progressWidgetTips->setTitle(tr("Importing images"));
    progressWidgetTips->setTips(
                QString(tr("%1 images imported, please wait")).arg(0));
    connect(dApp->importer, &Importer::importProgressChanged,
            [=](double per) {
        progressWidgetTips->setValue(int(per*100));
        progressWidgetTips->setTips(
                    QString(tr("%1 images imported, please wait"))
                    .arg(dApp->importer->finishedCount()));
    });
    connect(progressWidgetTips, &ProgressWidgetsTips::stopProgress, [=]{
        dApp->importer->stopImport();
        importProgressWidget->hide();
    });

    importProgressWidget->setContent(progressWidgetTips);
    //importProgress's tooltip end
    connect(dApp->importer, &Importer::importProgressChanged,
            this, [=] (double progress) {
        importProgress->setVisible(progress != 1);
        if (importProgress->isHidden()) {
            importProgressWidget->hide();
        }
        importProgress->setValue(progress * 100);
    });

    connect(importProgress, &DCircleProgress::clicked, [=]{
        dApp->importer->nap();
        if (importProgressWidget->isHidden()) {
            progressWidgetTips->show();
            importProgressWidget->show(window()->x()+window()->width() -
                                       importProgressWidget->width() / 2 - 6,
                                       window()->y() + 45);
        } else {
            importProgressWidget->hide();
        }
    });


    DWindowOptionButton *ob = new DWindowOptionButton;
    connect(ob, &DWindowOptionButton::clicked, this, [=] {
        if (parentWidget()) {
            m_popupMenu->setMenuContent(createMenuContent());
            m_popupMenu->showMenu();
        }
    });
    connect(dApp->signalM, &SignalManager::enableMainMenu,
            this, [=] (bool enable) {
        ob->setVisible(enable);
        ob->setEnabled(enable);
        });

    DWindowMinButton *minb = new DWindowMinButton;
    // FIXME it may crash
    connect(minb, SIGNAL(clicked()),
            parentWidget()->parentWidget(), SLOT(showMinimized()));

    m_maxb = new DWindowMaxButton;
    connect(m_maxb, &DWindowMaxButton::clicked, this, [=] {
        if (m_maxb->isMaximized()) {
            window()->showNormal();
            m_maxb->setMaximized(false);
        }
        else {
            window()->showMaximized();
            m_maxb->setMaximized(true);
        }
    });

    DWindowCloseButton *cb = new DWindowCloseButton;
    connect(cb, &DWindowCloseButton::clicked, qApp, &QApplication::quit);

    m_rightContent = new QWidget;
    QHBoxLayout *rightLayout = new QHBoxLayout(m_rightContent);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addStretch(1);
    rightLayout->addWidget(importProgress);
    rightLayout->addSpacing(38);
    rightLayout->addWidget(ob);
    rightLayout->addWidget(minb);
    rightLayout->addWidget(m_maxb);
    rightLayout->addWidget(cb);

    m_leftContent = new QWidget;
    m_leftLayout = new QHBoxLayout(m_leftContent);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(0);

    m_middleContent = new QWidget;
    m_middleLayout = new QHBoxLayout(m_middleContent);
    m_middleLayout->setContentsMargins(0, 0, 0, 0);
    m_middleLayout->setSpacing(0);

    mainLayout->addWidget(m_leftContent);
    mainLayout->addWidget(m_middleContent);
    mainLayout->addWidget(m_rightContent);
    mainLayout->addSpacing(ICON_MARGIN);
}

void TopToolbar::initMenu()
{
    m_popupMenu = new PopupMenuManager(this);
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &TopToolbar::onMenuItemClicked);
    m_popupMenu->setMenuContent(createMenuContent());
}

QString TopToolbar::createMenuContent()
{
    QJsonArray items;
    items.append(createMenuItem(IdCreateAlbum, tr("New album"), false,
                                "Ctrl+Shift+N"));
    items.append(createMenuItem(IdImport, tr("Import"), false, "Ctrl+I"));

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdHelp, tr("Help"), false, "F1"));
    items.append(createMenuItem(IdAbout, tr("About")));
    items.append(createMenuItem(IdQuick, tr("Exit"), false, "Ctrl+Q"));

    QJsonObject contentObj;
    const QPoint gp = this->mapToGlobal(QPoint(0, 0));
    const QSize ms = m_popupMenu->sizeHint();
    contentObj["x"] = gp.x() + width() - ms.width() - 14;
    contentObj["y"] = gp.y() + TOP_TOOLBAR_HEIGHT - 10;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

QJsonValue TopToolbar::createMenuItem(const MenuItemId id,
                                     const QString &text,
                                     const bool isSeparator,
                                     const QString &shortcut,
                                     const QJsonObject &subMenu)
{
    return QJsonValue(m_popupMenu->createItemObj(id,
                                                 text,
                                                 isSeparator,
                                                 shortcut,
                                                 subMenu));
}

void TopToolbar::onMenuItemClicked(int menuId, const QString &text)
{
    Q_UNUSED(text);

    switch (MenuItemId(menuId)) {
    case IdCreateAlbum:
        emit dApp->signalM->createAlbum();
        break;
    case IdImport:
        dApp->importer->showImportDialog();
        break;
    case IdHelp:
        showManual();
        break;
    case IdAbout:
        m_about->move((width() - m_about->width()) / 2 +
                      mapToGlobal(QPoint(0, 0)).x(),
                      (window()->height() - m_about->height()) / 2 +
                      mapToGlobal(QPoint(0, 0)).y());
        m_about->show();
        break;
    case IdQuick:
        qApp->quit();
    default:
        break;
    }
}

void TopToolbar::showManual()
{
    if (m_manualPro.isNull()) {
        const QString pro = "dman";
        const QStringList args("deepin-image-viewer");
        m_manualPro = new QProcess(this);
        connect(m_manualPro.data(), SIGNAL(finished(int)),
                m_manualPro.data(), SLOT(deleteLater()));
        m_manualPro->start(pro, args);
    }
}

void TopToolbar::showShortCutView() {
    QRect rect=window()->geometry();
    Shortcut sc;
    QStringList shortcutString;
    QString param1 = "-j="+sc.toStr();
    QString param2 = "-r="+
            QString::number(rect.x())+","+
            QString::number(rect.y())+","+
            QString::number(rect.width())+","+
            QString::number(rect.height());
    shortcutString<<param1<<param2;

    QProcess* shortcutViewProcess = new QProcess();
    shortcutViewProcess->startDetached("deepin-shortcut-viewer",shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)),
            shortcutViewProcess, SLOT(deleteLater()));
}
