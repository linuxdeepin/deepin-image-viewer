#include "toptoolbar.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "controller/viewerthememanager.h"
#include "settings/settingswindow.h"
#include "widgets/dialogs/aboutdialog.h"
#include "widgets/dialogs/cancelimportdialog.h"
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
#include <QMenu>
#include <QStyleFactory>

DWIDGET_USE_NAMESPACE

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int ICON_MARGIN = 6;

//const QColor DARK_COVERCOLOR = QColor(30, 30, 30, 204);
//const QColor LIGHT_COVERCOLOR = QColor(255, 255, 255, 230);

const QColor DARK_TOP_BORDERCOLOR = QColor(255, 255, 255, 13);
const QColor LIGHT_TOP_BORDERCOLOR = QColor(255, 255, 255, 153);

const QColor DARK_BOTTOM_BORDERCOLOR = QColor(0, 0, 0, 51);
const QColor LIGHT_BOTTOM_BORDERCOLOR = QColor(0, 0, 0, 26);
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
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    m_settingsWindow = new SettingsWindow();
    m_settingsWindow->hide();

    initWidgets();
    initMenu();
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

void TopToolbar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (window()->isMaximized())
            window()->showNormal();
        else if (! window()->isFullScreen())  // It would be normal state
            window()->showMaximized();
    }

    BlurFrame::mouseDoubleClickEvent(e);
}
void TopToolbar::onThemeChanged(ViewerThemeManager::AppTheme curTheme) {
    QLinearGradient lightLinearGrad;
    lightLinearGrad.setColorAt(0, QColor(255, 255, 255, 230));
    lightLinearGrad.setColorAt(1, QColor(248, 248, 248, 230));
    lightLinearGrad.setStart(x(), y());
    lightLinearGrad.setFinalStop(x(), y() + height());


    if (curTheme == ViewerThemeManager::Dark) {
        setCoverBrush(QBrush(QColor(30, 30, 30, 204)));
        m_topBorderColor = DARK_TOP_BORDERCOLOR;
        m_bottomBorderColor = DARK_BOTTOM_BORDERCOLOR;

        Dtk::Widget::DThemeManager::instance()->setTheme("dark");
    } else {
        setCoverBrush(QBrush(lightLinearGrad));
        m_topBorderColor = LIGHT_TOP_BORDERCOLOR;
        m_bottomBorderColor = LIGHT_BOTTOM_BORDERCOLOR;

        Dtk::Widget::DThemeManager::instance()->setTheme("light");
    }


    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TopToolbar::onThemeChanged);
}

const QString TopToolbar::newAlbumShortcut() const
{
    return dApp->setter->value("SHORTCUTALBUM", "New album", "Ctrl+Shift+N").toString();
}

void TopToolbar::paintEvent(QPaintEvent *e)
{
    BlurFrame::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw inside top border
    const QColor tc(m_topBorderColor);
    int borderHeight = 1;
    QPainterPath tPath;
    tPath.moveTo(QPointF(x(), y() + borderHeight - 0.5));
    tPath.lineTo(QPointF(x() + width(), y() + borderHeight - 0.5));
    p.setPen(QPen(tc));
    p.drawPath(tPath);
//    QPen tPen(tc);
//    QLinearGradient linearGrad;
//    linearGrad.setStart(x(), y());
//    linearGrad.setFinalStop(x() + width(), y());
//    linearGrad.setColorAt(0, Qt::transparent);
//    linearGrad.setColorAt(0.005, tc);
//    linearGrad.setColorAt(0.995, tc);
//    linearGrad.setColorAt(1, Qt::transparent);
//    tPen.setBrush(QBrush(linearGrad));
//    p.setPen(tPen);
//    p.drawPath(tPath);

    // Draw inside bottom border
//    QPainterPath bPath;
//    borderHeight = 0;
//    bPath.moveTo(x(), y() + height() - borderHeight - 0.5);
//    bPath.lineTo(x() + width(), y() + height() - borderHeight - 0.5);
//    QPen bPen(m_bottomBorderColor, borderHeight);
//    p.setPen(bPen);
//    p.drawPath(bPath);
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

    // Windows button
    DWindowOptionButton *optionBtn = new DWindowOptionButton;
    connect(optionBtn, &DWindowOptionButton::clicked, this, [=] {
        if (parentWidget()) {
            const QPoint gp = this->mapToGlobal(QPoint(0, 0));
            const QSize ms = m_menu->sizeHint();
            const QPoint p(gp.x() + width() - ms.width() - 30,
                           gp.y() + TOP_TOOLBAR_HEIGHT - 10);
            m_menu->popup(p);
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
    connect(closeBtn, &DWindowCloseButton::clicked, this, [=] {
        if (dApp->importer->isRunning()) {
            CancelImportDialog *cd = new CancelImportDialog;
            cd->show();
        }
        else {
            dApp->quit();
        }
    });

    m_rLayout->addWidget(optionBtn);
    m_rLayout->addWidget(minBtn);
    m_rLayout->addWidget(maxBtn);
    m_rLayout->addWidget(closeBtn);
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
    m_menu = new QMenu(this);
    m_menu->setStyle(QStyleFactory::create("dlight"));
    QAction *acNA = m_menu->addAction(tr("New album"));
    QAction *acDT = m_menu->addAction(tr("Deep color mode"));
    bool checkSelected =
            dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark;
    acDT->setCheckable(checkSelected);
    acDT->setChecked(checkSelected);
    QAction *acS = m_menu->addAction(tr("Setting"));
    m_menu->addSeparator();
    QAction *acH = m_menu->addAction(tr("Help"));
    QAction *acA = m_menu->addAction(tr("About"));
    QAction *acE = m_menu->addAction(tr("Exit"));
    connect(acNA, &QAction::triggered, this, &TopToolbar::onNewAlbum);
    connect(acDT, &QAction::triggered, this, &TopToolbar::onDeepColorMode);
    connect(acS, &QAction::triggered, this, &TopToolbar::onSetting);
    connect(acH, &QAction::triggered, this, &TopToolbar::onHelp);
    connect(acA, &QAction::triggered, this, &TopToolbar::onAbout);
    connect(acE, &QAction::triggered, dApp, &Application::quit);

    QShortcut *scNA = new QShortcut(QKeySequence(newAlbumShortcut()), this);
    QShortcut *scH = new QShortcut(QKeySequence("F1"), this);
    QShortcut *scE = new QShortcut(QKeySequence("Ctrl+Q"), this);
    QShortcut *scViewShortcut = new QShortcut(QKeySequence("Ctrl+Shift+/"), this);
    connect(scNA, SIGNAL(activated()), this, SLOT(onNewAlbum()));
    connect(scH, SIGNAL(activated()), this, SLOT(onHelp()));
    connect(scE, SIGNAL(activated()), dApp, SLOT(quit()));
    connect(scViewShortcut, SIGNAL(activated()), this, SLOT(onViewShortcut()));

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            [=](ViewerThemeManager::AppTheme dark){
        if (dark == ViewerThemeManager::Dark) {
            acDT->setCheckable(true);
            acDT->setChecked(true);
        } else {
            acDT->setCheckable(false);
            acDT->setChecked(false);
        }
    });

    connect(dApp->setter, &ConfigSetter::valueChanged, this, [=] (const QString &group) {
        if (group == "SHORTCUTALBUM") {
            scNA->setKey(QKeySequence(newAlbumShortcut()));
        }
    });
}

void TopToolbar::onViewShortcut() {
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

void TopToolbar::onAbout()
{
    AboutDialog *ad = new AboutDialog;
    ad->show();
}

void TopToolbar::onHelp()
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

void TopToolbar::onNewAlbum()
{
    emit dApp->signalM->createAlbum();
}

void TopToolbar::onDeepColorMode() {
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        dApp->viewerTheme->setCurrentTheme(
                    ViewerThemeManager::Light);
    } else {
        dApp->viewerTheme->setCurrentTheme(
                    ViewerThemeManager::Dark);
    }
}

void TopToolbar::onSetting()
{
    m_settingsWindow->move((width() - m_settingsWindow->width()) / 2 +
                           mapToGlobal(QPoint(0, 0)).x(),
                           (window()->height() - m_settingsWindow->height()) / 2 +
                           mapToGlobal(QPoint(0, 0)).y());
    m_settingsWindow->show();
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
    const QString ss = utils::base::getFileContent(":/resources/dark/qss/importtip.qss");
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
    cb->setNormalPic(":/resources/dark/images/importtip/close_normal.png");
    cb->setHoverPic(":/resources/dark/images/importtip/close_hover.png");
    cb->setPressPic(":/resources/dark/images/importtip/close_press.png");
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
