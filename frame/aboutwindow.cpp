#include "aboutwindow.h"
#include <dwindowclosebutton.h>
#include <QFile>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

using namespace Dtk::Widget;

AboutWindow::AboutWindow(QWidget *parent, QWidget *source)
    : BlureFrame(parent, source)
{
    setMinimumSize(380, 360);
    initStyleSheet();
    setBorderRadius(4);
    setBorderWidth(1);
    setBorderColor(QColor(255, 255, 255, 51));

    QLabel *appLogo = new QLabel();
    QPixmap ap = QPixmap(":/images/resources/images/deepin_image_viewer.png");
    ap.scaled(78, 78, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    appLogo->setPixmap(ap);

    QLabel *title = new QLabel(tr("Deepin Image Viewer"));
    title->setAlignment(Qt::AlignHCenter);
    title->setObjectName("AppTitleLabel");

    QLabel *version = new QLabel(tr("Version: ") + "0.1.0");
    version->setAlignment(Qt::AlignHCenter);
    version->setObjectName("VersionLabel");

    QLabel *corpLogo = new QLabel();
    QPixmap cp(":/images/resources/images/deepin_logo.png");
    cp.scaled(96, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    corpLogo->setPixmap(cp);

    QLabel *link = new QLabel();
    link->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    link->setText("<a href=\"https://www.deepin.org/\">www.deepin.org</a>");
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

    layout->addSpacing(23);
    layout->addWidget(appLogo, 0, Qt::AlignHCenter);
    layout->addSpacing(7);
    layout->addWidget(title);
    layout->addSpacing(6);
    layout->addWidget(version);
    layout->addSpacing(18);
    layout->addWidget(corpLogo, 0, Qt::AlignHCenter);
    layout->addSpacing(6);
    layout->addWidget(link);
    layout->addSpacing(20);
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
