#include "EditPanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>
#include "controller/signalmanager.h"
#include "FilterSetup.h"
#include "filters/FilterObj.h"

using namespace Dtk::Widget;

EditPanel::EditPanel(QWidget *parent)
    : ModulePanel(parent)
{
    connect(SignalManager::instance(), &SignalManager::editImage, this, &EditPanel::openImage);
    m_view = new ImageWidget();
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_view);
    m_filterSetup = new FilterSetup(this);
    connect(m_filterSetup, &FilterSetup::filterIdChanged, this, &EditPanel::setFilterId);
    connect(m_filterSetup, &FilterSetup::filterIndensityChanged, this, &EditPanel::setFilterIndensity);
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

void EditPanel::setFilterId(int value)
{
    if (m_filterId == value)
        return;
    m_filterId = value;
    if (m_filter) {
        delete m_filter;
        m_filter = 0;
    }
    qDebug("filter id: %d", m_filterId);
    m_filter = filter2d::FilterObj::create(m_filterId);
    applyFilter();
}

void EditPanel::setFilterIndensity(qreal value)
{
    if (m_filterIndensity == value)
        return;
    m_filterIndensity = value;
    applyFilter();
}

void EditPanel::applyFilter()
{
    if (!m_filter)
        return;
    m_filter->setProperty("brightness", 0.6);
    m_filter->setProperty("hue", 0.6);
    m_filter->setProperty("contrast", 0.6);
    m_filter->setProperty("saturation", 0.6);
    qDebug("set indensity: %.3f", m_filterIndensity);
    m_filter->setIndensity(m_filterIndensity);
    QImage img(m_image);
    if (img.isNull())
        return;
    m_view->setImage(m_filter->apply(img));
}

void EditPanel::openImage(const QString &path)
{
    m_image = QImage(path);
    Q_EMIT SignalManager::instance()->gotoPanel(this);
    m_view->setImage(m_image);
    m_filterSetup->resize(240, height() - 30);
    m_filterSetup->show();
    m_filterSetup->setImage(path);
}
