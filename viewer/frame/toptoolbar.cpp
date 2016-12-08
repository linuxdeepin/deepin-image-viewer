#include "toptoolbar.h"
#include "application.h"
#include "controller/importer.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "frame/mainwindow.h"
#include "settings/settingswindow.h"
#include "widgets/progresswidgetstips.h"
#include "widgets/dialogs/aboutdialog.h"
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

TopToolbar::TopToolbar(QWidget *parent)
    :BlurFrame(parent)
{
//    QLinearGradient linearGrad(QPoint(0, this->y()),
//                               QPoint(0, this->y()+this->height()));
//    linearGrad.setColorAt(0, QColor(38, 38, 38, 230));
//    linearGrad.setColorAt(1, QColor(28, 28, 28, 230));

//    setCoverBrush(QBrush(linearGrad));
    setCoverBrush(QBrush(QColor(30, 30, 30, 204)));

    m_settingsWindow = new SettingsWindow();
    m_settingsWindow->hide();

    initWidgets();
    initMenu();

    QShortcut* viewScut = new QShortcut(QKeySequence("Ctrl+Shift+/"),
                                        this);
    connect(viewScut, &QShortcut::activated, this, &TopToolbar::showShortCutView);
    qApp->installEventFilter(this);
    parent->installEventFilter(this);
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
    if (e->type() == QEvent::Move && obj == this) {
        emit updateImportTipsGeo();
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
    Q_UNUSED(e);
    emit dApp->signalM->updateTopToolbar();
    m_leftContent->setFixedWidth((window()->width() - 48*6)/2);
    m_middleContent->update();
    m_rightContent->setFixedWidth((window()->width() - 48*6)/2);
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
    BlurFrame::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw inside top border
    const QColor tc(255, 255, 255, 20);
    int borderHeight = 1;
    QPainterPath tPath;
    tPath.moveTo(QPointF(x(), y() + borderHeight - 0.5));
    tPath.lineTo(QPointF(x() + width(), y() + borderHeight - 0.5));

    QPen tPen(tc);
    QLinearGradient linearGrad;
    linearGrad.setStart(x(), y());
    linearGrad.setFinalStop(x() + width(), y());
    linearGrad.setColorAt(0, Qt::transparent);
    linearGrad.setColorAt(0.005, tc);
    linearGrad.setColorAt(0.995, tc);
    linearGrad.setColorAt(1, Qt::transparent);
    tPen.setBrush(QBrush(linearGrad));
    p.setPen(tPen);
    p.drawPath(tPath);

    // Draw inside bottom border
    QPainterPath bPath;
    borderHeight = 0;
    bPath.moveTo(x(), y() + height() - borderHeight - 0.5);
    bPath.lineTo(x() + width(), y() + height() - borderHeight - 0.5);
    QPen bPen(QColor(0, 0, 0, 51), borderHeight);
    p.setPen(bPen);
    p.drawPath(bPath);
}

void TopToolbar::initWidgets()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    DCircleProgress *importDCProg = new DCircleProgress(this);
    importDCProg->setValue(0);
    importDCProg->setFixedSize(21, 21);
    importDCProg->setVisible(false);
    //importProgress's tooltip begin to init;
    DArrowRectangle *importDArrowRect =
            new DArrowRectangle(DArrowRectangle::ArrowTop, nullptr);

    importDArrowRect->setWindowFlags(Qt::X11BypassWindowManagerHint |
                                     Qt::WindowStaysOnTopHint);

    importDArrowRect->setStyleSheet("background-color:red;");
    importDArrowRect->setShadowBlurRadius(0);
    importDArrowRect->setShadowDistance(0);
    importDArrowRect->setShadowYOffset(0);
    importDArrowRect->setShadowXOffset(0);

    ProgressWidgetsTips* progTips = new ProgressWidgetsTips(/*importDArrowRect*/);
    progTips->setTitle(tr("Collecting information"));
    progTips->setTips(
                QString(tr("%1 folders has been collected, please wait")).arg(0));
    importDArrowRect->setContent(progTips);
    importDArrowRect->hide();
    connect(dApp->importer, &Importer::progressChanged,
            [=](double per, int count) {
        progTips->setValue(int(per*100));
        progTips->setTips(
                    QString(tr("%1 folders has been collected, please wait"))
                    .arg(count));
    });
    connect(progTips, &ProgressWidgetsTips::stopProgress, [=]{
        dApp->importer->cancel();
        progTips->hide();
    });

    connect(dApp->importer, &Importer::imported, [=](bool succeess){
        if (succeess && importDArrowRect->isVisible()) {
            importDArrowRect->hide();
        }
    });


    //importProgress's tooltip end
    connect(dApp->importer, &Importer::progressChanged,
            this, [=] (double progress) {
        importDCProg->setVisible(progress != 1);
        if (progress == 1) {
            importDArrowRect->hide();
        }
        importDCProg->setValue(progress * 100);
    });

    connect(importDCProg, &DCircleProgress::clicked, [=]{
        if (importDArrowRect->isHidden()) {
            importDArrowRect->setContent(progTips);
            importDArrowRect->show(window()->width() + window()->x() -
             161, mapToGlobal(QPoint(0, 45)).y());
        } else {
            importDArrowRect->hide();
        }
    });

    connect(dApp->signalM, &SignalManager::updateTopToolbar, this, [=]{
        if (!importDArrowRect->isHidden()) {
            importDArrowRect->hide();
            if (importDCProg->isVisible()) {
                QTimer* delayShowTimer = new QTimer(this);
                delayShowTimer->start(2000);
                connect(delayShowTimer, &QTimer::timeout, [=]{
                    importDArrowRect->setContent(progTips);
                    importDArrowRect->show(window()->width() + window()->x() -
                                           161, mapToGlobal(QPoint(0, 45)).y());
                    delayShowTimer->deleteLater();
                });
            }
        }
    });

    connect(this, &TopToolbar::updateImportTipsGeo, this, [=]{
        if (!importDArrowRect->isHidden()) {
            importDArrowRect->hide();
            if (importDCProg->isVisible()) {
                QTimer* delayShowTimer = new QTimer(this);
                delayShowTimer->start(2000);
                connect(delayShowTimer, &QTimer::timeout, [=]{
                    importDArrowRect->setContent(progTips);
                    importDArrowRect->show(window()->width() + window()->x() -
                                           161, mapToGlobal(QPoint(0, 45)).y());
                    delayShowTimer->deleteLater();
                });
            }
        }
    });
    connect(dApp, &DApplication::focusChanged, this, [=]{
        if (!window()->hasFocus() && !importDArrowRect->isHidden()) {
            importDArrowRect->hide();
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
    connect(minb,  &DWindowMinButton::clicked, this, [=]{
        if (!importDArrowRect->isHidden()) {
            importDArrowRect->hide();
        }
    });

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
    m_rightContent->setFixedWidth((window()->width() - 48*6)/2);
    QHBoxLayout *rightLayout = new QHBoxLayout(m_rightContent);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addStretch();
    rightLayout->addWidget(importDCProg);
    rightLayout->addSpacing(36);
    rightLayout->addWidget(ob);
    rightLayout->addSpacing(0);
    rightLayout->addWidget(minb);
    rightLayout->addSpacing(0);
    rightLayout->addWidget(m_maxb);
    rightLayout->addSpacing(0);
    rightLayout->addWidget(cb);
    rightLayout->addSpacing(ICON_MARGIN);

    m_leftContent = new QWidget;
    m_leftContent->setFixedWidth((window()->width() - 48*6)/2);
    m_leftLayout = new QHBoxLayout(m_leftContent);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(0);

    m_middleContent = new QWidget;
    m_middleLayout = new QHBoxLayout(m_middleContent);
    m_middleLayout->setContentsMargins(0, 0, 0, 0);
    m_middleLayout->setSpacing(0);

    mainLayout->addWidget(m_leftContent);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_middleContent);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_rightContent);
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
    items.append(createMenuItem(IdSetting, tr("Setting"), false, "Ctrl+I"));

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
    case IdSetting:
        m_settingsWindow->move((width() - m_settingsWindow->width()) / 2 +
                               mapToGlobal(QPoint(0, 0)).x(),
                               (window()->height() - m_settingsWindow->height()) / 2 +
                               mapToGlobal(QPoint(0, 0)).y());
        m_settingsWindow->show();
        break;
    case IdHelp:
        showManual();
        break;
    case IdAbout:{
        AboutDialog *ad = new AboutDialog;
        ad->show();
        break;}
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
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width()/2 , rect.y() + rect.height()/2);
    Shortcut sc;
    QStringList shortcutString;
    QString param1 = "-j="+sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess* shortcutViewProcess = new QProcess();
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)),
            shortcutViewProcess, SLOT(deleteLater()));
}
