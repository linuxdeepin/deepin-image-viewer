#include "mainwidget.h"
#include <dimagebutton.h>
#include <QFile>
#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>

using namespace Dtk::Widget;

const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 24;

MainWidget::MainWidget(QWidget *parent)
    : QFrame(parent)
{
    initStyleSheet();
    setMinimumSize(700, 500);

    initCenterContent();
    initTopToolbar();
    initBottomToolbar();
}

MainWidget::~MainWidget()
{

}

void MainWidget::resizeEvent(QResizeEvent *)
{
    if (m_TopToolbar) {
        updateTopToolbarPosition();
    }
    if (m_BottomToolbar) {
        updateBottomToolbarPosition();
    }
}

void MainWidget::initCenterContent()
{
    m_centerContent = new QWidget(this);
    m_centerContent->setObjectName("CenterContent");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_centerContent);
}

void MainWidget::initTopToolbar()
{
    m_TopToolbar = new TopToolbar(this, m_centerContent);
    updateTopToolbarPosition();
}

void MainWidget::initBottomToolbar()
{
    m_BottomToolbar = new BottomToolbar(this, m_centerContent);
    updateBottomToolbarPosition();
}

void MainWidget::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/default.qss");
    if (!sf.open(QIODevice::ReadOnly)) {
        qWarning() << "Open style-sheet file error:" << sf.errorString();
        return;
    }

    qApp->setStyleSheet(QString(sf.readAll()));
    sf.close();
}

void MainWidget::updateTopToolbarPosition()
{
    m_TopToolbar->resize(width(), TOP_TOOLBAR_HEIGHT);
    m_TopToolbar->move(0, 0);
}

void MainWidget::updateBottomToolbarPosition()
{
    m_BottomToolbar->resize(width(), BOTTOM_TOOLBAR_HEIGHT);
    m_BottomToolbar->move(0, height() - BOTTOM_TOOLBAR_HEIGHT);
}
