#include "toptoolbar.h"
#include "controller/importer.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "controller/importer.h"
#include "frame/mainwindow.h"
#include <dcircleprogress.h>
#include "dwindowmaxbutton.h"
#include <dwindowminbutton.h>
#include <dwindowclosebutton.h>
#include <dwindowoptionbutton.h>
#include <dwindowrestorebutton.h>
#include <QDebug>
#include <QGradient>
#include <QResizeEvent>
#include <QApplication>
#include <QStackedWidget>

using namespace Dtk::Widget;

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;

}  // namespace

TopToolbar::TopToolbar(QWidget *parent, QWidget *source)
    :BlureFrame(parent, source)
{
    QLinearGradient linearGrad;
    linearGrad.setColorAt(0, QColor(0, 0, 0, 178));
    linearGrad.setColorAt(1, QColor(0, 0, 0, 204));

    setCoverBrush(QBrush(linearGrad));

    m_about = new AboutWindow(parent, source);
    m_about->hide();

    initWidgets();
    initMenu();

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

void TopToolbar::resizeEvent(QResizeEvent *e)
{
    m_leftContent->setFixedWidth(e->size().width() / 3);
    m_middleContent->setFixedWidth(e->size().width() / 3);
    m_rightContent->setFixedWidth(e->size().width() / 3);
}

void TopToolbar::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit moving();
}

void TopToolbar::initWidgets()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    DCircleProgress *importProgress = new DCircleProgress;
    importProgress->setValue(0);
    importProgress->setFixedSize(21, 21);
    importProgress->setVisible(false);
    connect(Importer::instance(), &Importer::importProgressChanged,
            this, [=] (double progress) {
        importProgress->setVisible(progress != 1);
        importProgress->setValue(progress * 100);
    });

    DWindowOptionButton *ob = new DWindowOptionButton;
    connect(ob, &DWindowOptionButton::clicked, this, [=] {
        if (parentWidget()) {
            m_popupMenu->setMenuContent(createMenuContent());
            m_popupMenu->showMenu();
        }
    });
    DWindowMinButton *minb = new DWindowMinButton;
    connect(minb, SIGNAL(clicked()), parentWidget()->parentWidget(), SLOT(showMinimized()));

    QStackedWidget *sw = new QStackedWidget;
    DWindowMaxButton *maxb = new DWindowMaxButton;

    connect(maxb, &DWindowMaxButton::clicked, this, [=] {
        if (parentWidget()) {
            emit maxiWindow();
            sw->setCurrentIndex(1);

        }
    });
    DWindowRestoreButton *rb = new DWindowRestoreButton;
    connect(rb, &DWindowRestoreButton::clicked, this, [=] {
        if (parentWidget()) {
            emit maxiWindow();
            sw->setCurrentIndex(0);
        }
    });
    sw->addWidget(maxb);
    sw->addWidget(rb);
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
    rightLayout->addWidget(sw);
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
    contentObj["y"] = gp.y() + TOP_TOOLBAR_HEIGHT + 14;
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
        emit SignalManager::instance()->createAlbum();
        break;
    case IdImport:
        Importer::instance()->showImportDialog();
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
        connect(m_manualPro.data(), SIGNAL(finished(int)), m_manualPro.data(), SLOT(deleteLater()));
        m_manualPro->start(pro, args);
    }
}
