#include "viewerthememanager.h"
#include "application.h"
#include "configsetter.h"

namespace {
const QString THEME_GROUP = "APP";
const QString THEME_TEXT = "AppTheme";
}

ViewerThemeManager * ViewerThemeManager::m_viewerTheme = NULL;
ViewerThemeManager *ViewerThemeManager::instance()
{
    if (m_viewerTheme == NULL) {
        m_viewerTheme = new ViewerThemeManager;
    }

    return m_viewerTheme;
}

ViewerThemeManager::ViewerThemeManager(QObject *parent) : QObject(parent)
{
}

ViewerThemeManager::AppTheme ViewerThemeManager::getCurrentTheme(){
    return m_currentTheme;
}

void ViewerThemeManager::setCurrentTheme(AppTheme theme) {
    m_currentTheme = theme;
    if (m_currentTheme == Dark)
        dApp->setter->setValue(THEME_GROUP, THEME_TEXT, QVariant("Dark"));
    else
        dApp->setter->setValue(THEME_GROUP, THEME_TEXT, QVariant("Light"));
    emit viewerThemeChanged(m_currentTheme);
}
