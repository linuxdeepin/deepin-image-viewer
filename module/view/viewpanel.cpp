#include "viewpanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>
#include "controller/signalmanager.h"

using namespace Dtk::Widget;

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
{
    connect(SignalManager::instance(), &SignalManager::viewImage, this, &ViewPanel::openImage);
    m_view = new ImageWidget();
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_view);
}

QWidget *ViewPanel::toolbarBottomContent()
{
    return NULL;
}

QWidget *ViewPanel::toolbarTopLeftContent()
{
    return NULL;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    return NULL;
}

QWidget *ViewPanel::extensionPanelContent()
{
    return NULL;
}

void ViewPanel::openImage(const QString &path)
{
    Q_EMIT SignalManager::instance()->gotoPanel(this);
    m_view->setImage(path);
}
