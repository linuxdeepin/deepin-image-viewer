#include "bottomtoolbar.h"
#include "application.h"
namespace {
const QColor DARK_COVERCOLOR = QColor(30, 30, 30, 204);
const QColor LIGHT_COVERCOLOR = QColor(255, 255, 255, 230);
}
BottomToolbar::BottomToolbar(QWidget *parent)
    : BlurFrame(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &BottomToolbar::onThemeChanged);
}

void BottomToolbar::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_coverBrush = DARK_COVERCOLOR;
    } else {
        m_coverBrush = LIGHT_COVERCOLOR;
    }
    setCoverBrush(m_coverBrush);
}
void BottomToolbar::setContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_mainLayout->addWidget(content);
}

void BottomToolbar::mouseMoveEvent(QMouseEvent *)
{

}
