#include "loadingicon.h"
#include "application.h"
#include "controller/viewerthememanager.h"

LoadingIcon::LoadingIcon(QWidget *parent)
    : DPictureSequenceView(parent)
{
    updateIconPath();
    setPictureSequence(m_iconPaths);
    setFixedSize(14, 14);
    setSpeed(20);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &LoadingIcon::updateIconPath);
}

void LoadingIcon::updateIconPath()
{
    QString iconPath;
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        iconPath = ":/images/loadings/resources/dark/images/dark_loading/loading_%1.png";
    }
    else {
        iconPath = ":/images/loadings/resources/dark/images/white_loading/loading_%1.png";
    }
    m_iconPaths.clear();

    for (int i = 1; i < 45; ++i)
        m_iconPaths.append(iconPath.arg(i));
}
