#include "settingswindow.h"
#include "titleframe.h"
#include "contentsframe.h"
#include "utils/baseutils.h"
#include <DTitlebar>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    :DMainWindow(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setStyleSheet(utils::base::getFileContent(
                      ":/settings/qss/resources/qss/settings.qss"));

    if (titleBar()) titleBar()->setFixedHeight(0);
    setFixedSize(680, 560);

    // Main
    QFrame *f = new QFrame;
    setCentralWidget(f);
    QVBoxLayout *mainLayout = new QVBoxLayout(f);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Contents
    QHBoxLayout *cbl = new QHBoxLayout;
    cbl->setContentsMargins(0, 0, 0, 0);
    cbl->setSpacing(0);

    // TitleFrame
    TitleFrame *tf = new TitleFrame;
    cbl->addWidget(tf);
    ContentsFrame *cf = new ContentsFrame(this);
    cbl->addWidget(cf);

    connect(tf, &TitleFrame::clicked,
            cf, &ContentsFrame::setCurrentID);
    connect(cf, &ContentsFrame::currentFieldChanged,
            tf, &TitleFrame::setCurrentID);

    mainLayout->addLayout(cbl);
}

void SettingsWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }

    DMainWindow::keyPressEvent(e);
}
