#include "aboutwindow.h"
#include <dwindowclosebutton.h>
#include <QFile>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QPainter>
using namespace Dtk::Widget;

AboutWindow::AboutWindow(QWidget *parent, QWidget *source)
    : BlureFrame(parent, source)
{
    setMinimumSize(380, 347);
    setBlurBackground(false);
    initStyleSheet();
    setBorderRadius(4);
    setBorderWidth(1);
    setBorderColor(QColor(255, 255, 255, 51));

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Sheet | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);

    QLabel *appLogo = new QLabel();
    QString logoPath = ":/images/resources/images/deepin_image_viewer.svg";
    QSvgRenderer renderer(logoPath);
    QPixmap ap = QPixmap(":/images/resources/images/deepin_image_viewer.svg");
    ap.fill(Qt::transparent);
    ap = ap.scaled(78, 78, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPainter painter;
    painter.begin(&ap);
    renderer.render(&painter);
    painter.end();
    appLogo->setPixmap(ap);

    QLabel *title = new QLabel(tr("Deepin Image Viewer"));
    title->setAlignment(Qt::AlignHCenter);
    title->setObjectName("AppTitleLabel");

    QLabel *version = new QLabel(tr("Version: ") + "1.0");
    version->setAlignment(Qt::AlignHCenter);
    version->setObjectName("VersionLabel");

    QLabel *corpLogo = new QLabel();
    QPixmap cp(":/images/resources/images/deepin_logo.png");
    cp.scaled(96, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    corpLogo->setPixmap(cp);

    QLabel *link = new QLabel();
    link->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    link->setText("<style>a{text-decoration: none;} </style><a href=\"https://www.deepin.org/\">"
                  "<font color='#0066ec';font size=12px;>www.deepin.org</a>");

    link->setTextFormat(Qt::RichText);
    link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    link->setOpenExternalLinks(true);
    link->setFixedHeight(32);

    QLabel *desc1 = new QLabel(
                tr("Deepin Image Viewer is a fashion & smooth image manager."));
    desc1->setAlignment(Qt::AlignHCenter);
    desc1->setWordWrap(true);
    desc1->setObjectName("DescLabel1");
    desc1->setMinimumWidth(380);

    QLabel *desc2 = new QLabel(tr("It is featured with image management, "
                                  "image viewing and basic image editing."));
    desc2->setAlignment(Qt::AlignHCenter);
    desc2->setWordWrap(true);
    desc2->setObjectName("DescLabel2");
    desc2->setContentsMargins(10, 0, 10, 0);

    QLabel *license = new QLabel(
                tr("Deepin Image Viewer is released under GPL v3."));
    license->setAlignment(Qt::AlignHCenter);
    license->setObjectName("LicenseLabel");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignHCenter);

    layout->addStretch(1);
    layout->addWidget(appLogo, 0, Qt::AlignHCenter);
    layout->addSpacing(-1);
    layout->addWidget(title);
    layout->addSpacing(4);
    layout->addWidget(version);
    layout->addSpacing(18);
    layout->addWidget(corpLogo, 0, Qt::AlignHCenter);
    layout->addSpacing(-6);
    layout->addWidget(link);
    layout->addSpacing(18);
    layout->addWidget(desc1);
    layout->addSpacing(6);
    layout->addWidget(desc2);
    layout->addSpacing(9);
    layout->addWidget(license);
    layout->addStretch(1);

    DWindowCloseButton *cb = new DWindowCloseButton(this);
    cb->setFixedSize(24, 24);
    cb->move(layout->sizeHint().width() - cb->width() - 8, 8);
    connect(cb, &DWindowCloseButton::clicked, this, &AboutWindow::close);
}

void AboutWindow::initStyleSheet()
{
    QFile f(":/qss/resources/qss/AboutWindow.qss");
    if (f.open(QIODevice::ReadOnly)) {
        setStyleSheet(f.readAll());
        f.close();
    }
}
