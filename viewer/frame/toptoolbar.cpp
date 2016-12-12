#include "toptoolbar.h"
#include "application.h"
#include "controller/importer.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "settings/settingswindow.h"
#include "widgets/dialogs/aboutdialog.h"
#include "utils/baseutils.h"
#include "utils/shortcut.h"
#include <darrowrectangle.h>
#include <dcircleprogress.h>
#include <dwindowminbutton.h>
#include <dwindowmaxbutton.h>
#include <dwindowclosebutton.h>
#include <dwindowoptionbutton.h>
#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QProcess>
#include <QResizeEvent>
#include <QShortcut>

DWIDGET_USE_NAMESPACE

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int ICON_MARGIN = 6;

}  // namespace

class ImportTip : public DArrowRectangle
{
    Q_OBJECT
public:
    explicit ImportTip(QWidget * parent = 0);
    void setValue(int value);
    void setTitle(QString title);
    void setMessage(QString message);

signals:
    void stopProgress();

private:
    void initWidgets();

private:
    DCircleProgress *m_cp;
    QLabel *m_title;
    QLabel *m_message;
    QWidget *m_content;
};

TopToolbar::TopToolbar(QWidget *parent)
    :BlurFrame(parent)
{
    setCoverBrush(QBrush(QColor(30, 30, 30, 204)));

    m_settingsWindow = new SettingsWindow();
    m_settingsWindow->hide();

    initWidgets();
    initMenu();

    QShortcut* viewScut = new QShortcut(QKeySequence("Ctrl+Shift+/"), this);
    connect(viewScut, &QShortcut::activated, this, &TopToolbar::showShortCutView);
}

void TopToolbar::setLeftContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_lLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_lLayout->addWidget(content);
}

void TopToolbar::setMiddleContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_mLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_mLayout->addWidget(content);
}

void TopToolbar::resizeEvent(QResizeEvent *e)
{
    BlurFrame::resizeEvent(e);
    updateTipsPos();
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

void TopToolbar::keyPressEvent(QKeyEvent *e)
{
    BlurFrame::keyPressEvent(e);
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke && ke->key() == Qt::Key_F1) {
            showManual();
        }
    }
}

void TopToolbar::initLeftContent()
{
    QWidget *w = new QWidget;
    m_lLayout = new QHBoxLayout(w);
    m_lLayout->setContentsMargins(0, 0, 0, 0);
    m_lLayout->setSpacing(0);

    m_layout->addWidget(w, 1, Qt::AlignLeft);
}

void TopToolbar::initMiddleContent()
{
    QWidget *w = new QWidget;
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mLayout = new QHBoxLayout(w);
    m_mLayout->setContentsMargins(0, 0, 0, 0);
    m_mLayout->setSpacing(0);

    m_layout->addWidget(w);
}

void TopToolbar::initRightContent()
{
    QWidget *w = new QWidget;
    m_rLayout = new QHBoxLayout(w);
    m_rLayout->setContentsMargins(0, 0, 0, 0);
    m_rLayout->setSpacing(0);

    m_layout->addWidget(w, 1, Qt::AlignRight);

    // Collection progress
    initImportTips();
    DCircleProgress *cp = new DCircleProgress(this);
    cp->setValue(0);
    cp->setFixedSize(21, 21);
    cp->setVisible(false);
    connect(dApp->importer, &Importer::progressChanged, this, [=] (double p) {
        cp->setVisible(p != 1);
        if (p == 1) {
            m_importTips->hide();
        }
        cp->setValue(p * 100);
    });
    connect(cp, &DCircleProgress::clicked, [=]{
        if (m_importTips->isHidden()) {
            m_importTips->setVisible(true);
            m_importTips->setFocus();
            updateTipsPos();
        }
        else {
            m_importTips->hide();
        }
    });

    // Windows button
    DWindowOptionButton *optionBtn = new DWindowOptionButton;
    connect(optionBtn, &DWindowOptionButton::clicked, this, [=] {
        if (parentWidget()) {
            m_popupMenu->setMenuContent(createMenuContent());
            m_popupMenu->showMenu();
        }
    });
    connect(dApp->signalM, &SignalManager::enableMainMenu, this, [=] (bool v) {
        optionBtn->setVisible(v);
        optionBtn->setEnabled(v);
    });
    DWindowMinButton *minBtn = new DWindowMinButton;
    connect(minBtn, &DWindowMinButton::clicked, this, [=] {
        if (parentWidget() && parentWidget()->parentWidget()) {
            parentWidget()->parentWidget()->showMinimized();
        }
        m_importTips->hide();
    });
    DWindowMaxButton *maxBtn = new DWindowMaxButton;
    connect(maxBtn, &DWindowMaxButton::clicked, this, [=] {
        if (maxBtn->isMaximized()) {
            window()->showNormal();
            maxBtn->setMaximized(false);
        }
        else {
            window()->showMaximized();
            maxBtn->setMaximized(true);
        }
    });
    DWindowCloseButton *closeBtn = new DWindowCloseButton;
    connect(closeBtn, &DWindowCloseButton::clicked, qApp, &QApplication::quit);

    m_rLayout->addWidget(cp);
    m_rLayout->addWidget(optionBtn);
    m_rLayout->addWidget(minBtn);
    m_rLayout->addWidget(maxBtn);
    m_rLayout->addWidget(closeBtn);
}

void TopToolbar::initImportTips()
{
    QString tipStr = tr("%1 folders has been collected, please wait");
    m_importTips = new ImportTip;
    m_importTips->setTitle(tr("Collecting information"));
    m_importTips->setMessage(tipStr.arg(0));
    m_importTips->hide();

    connect(dApp, &DApplication::focusChanged, this, [=]{
        if (! window()->hasFocus() && ! m_importTips->hasFocus()) {
//            TIMER_SINGLESHOT(1000, {m_importTips->hide();}, this)
            m_importTips->hide();
        }
    });
    connect(dApp->importer, &Importer::progressChanged,
            [=](double per, int count) {
        m_importTips->setValue(int(per*100));
        m_importTips->setMessage(tipStr.arg(count));
    });
    connect(dApp->importer, &Importer::imported, [=](bool succeess){
        if (succeess && m_importTips->isVisible()) {
            m_importTips->hide();
        }
    });
    connect(m_importTips, &ImportTip::stopProgress, [=]{
        dApp->importer->cancel();
        m_importTips->hide();
    });
}

void TopToolbar::initWidgets()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    initLeftContent();
    initMiddleContent();
    initRightContent();
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

void TopToolbar::updateTipsPos()
{
    m_importTips->move(window()->width() + window()->x() - m_importTips->width() / 2 + 42,
                       mapToGlobal(QPoint(0, 0)).y() + TOP_TOOLBAR_HEIGHT - 10);
}

ImportTip::ImportTip(QWidget *parent)
    :DArrowRectangle(DArrowRectangle::ArrowTop, parent)
{
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setShadowBlurRadius(0);
    setShadowDistance(0);
    setShadowYOffset(0);
    setShadowXOffset(0);
    setMargin(0);

    initWidgets();
}

void ImportTip::setValue(int value)
{
    m_cp->setValue(value);
}

void ImportTip::setTitle(QString title)
{
    m_title->setText(title);
}

void ImportTip::setMessage(QString message)
{
    m_message->setText(message);
    int width = utils::base::stringWidth(m_message->font(), m_message->text());
    m_message->setMinimumWidth(width);

    // set content to force recalculate the size of content
    setContent(m_content);
}

void ImportTip::initWidgets()
{
    const QString ss = utils::base::getFileContent(":/qss/resources/qss/importtip.qss");
    m_cp = new DCircleProgress(this);
    m_cp->setFixedSize(32, 32);
    m_cp->setValue(0);

    m_title = new QLabel(this);
    m_title->setObjectName("ProgressDialogTitle");
    m_title->setStyleSheet(ss);
    m_message = new QLabel(this);
    m_message->setObjectName("ProgressDialogMessage");
    m_message->setStyleSheet(ss);

    QVBoxLayout* contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0 ,0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch(1);
    contentLayout->addWidget(m_title);
//    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_message);
    contentLayout->addStretch(1);

    DImageButton *cb = new DImageButton(this);
    cb->setNormalPic(":/images/importtip/resources/images/close_normal.png");
    cb->setHoverPic(":/images/importtip/resources/images/close_hover.png");
    cb->setPressPic(":/images/importtip/resources/images/close_press.png");
    connect(cb, &DImageButton::clicked, this, &ImportTip::stopProgress);

    m_content = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(m_content);
    layout->setContentsMargins(15, 13, 11, 12);
    layout->addWidget(m_cp);
    layout->addSpacing(10);
    layout->addLayout(contentLayout);
    layout->addSpacing(80);
    layout->addWidget(cb);
}

#include "toptoolbar.moc"
