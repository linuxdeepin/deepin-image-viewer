#include "EditPanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>
#include "controller/signalmanager.h"

using namespace Dtk::Widget;

EditPanel::EditPanel(QWidget *parent)
    : ModulePanel(parent)
{
    connect(SignalManager::instance(), &SignalManager::editImage, this, &EditPanel::openImage);
    m_view = new ImageWidget();
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_view);
}

QWidget *EditPanel::toolbarBottomContent()
{
    return NULL;
}

QWidget *EditPanel::toolbarTopLeftContent()
{
    return NULL;
}

QWidget *EditPanel::toolbarTopMiddleContent()
{
    return NULL;
}

QWidget *EditPanel::extensionPanelContent()
{
    return NULL;
}

void EditPanel::updateToolbarContent()
{
    emit updateTopToolbarLeftContent(toolbarTopLeftContent());
    emit updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    emit updateBottomToolbarContent(toolbarBottomContent());
}

void EditPanel::openImage(const QString &path)
{
    Q_EMIT SignalManager::instance()->gotoPanel(this);
    m_view->setImage(path);
}
