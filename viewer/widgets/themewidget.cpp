#include "themewidget.h"
#include "utils/baseutils.h"

#include "application.h"

ThemeWidget::ThemeWidget(const QString &darkFile, const QString &lightFile,
                         QWidget *parent)
    : QWidget(parent) {

    m_darkStyle = utils::base::getFileContent(darkFile);
    m_lightStyle = utils::base::getFileContent(lightFile);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThemeWidget::onThemeChanged);
}

ThemeWidget::~ThemeWidget() {}

void ThemeWidget::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(m_darkStyle);
    } else {
        this->setStyleSheet(m_lightStyle);
    }
}

ThemeScrollArea::ThemeScrollArea(const QString &darkFile, const QString &lightFile,
                         QWidget *parent)
    : QScrollArea(parent) {

    m_darkStyle = utils::base::getFileContent(darkFile);
    m_lightStyle = utils::base::getFileContent(lightFile);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThemeScrollArea::onThemeChanged);
}

ThemeScrollArea::~ThemeScrollArea() {}

void ThemeScrollArea::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(m_darkStyle);
    } else {
        this->setStyleSheet(m_lightStyle);
    }
}
