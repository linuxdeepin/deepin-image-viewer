#ifndef VIEWERTHEMEMANAGER_H
#define VIEWERTHEMEMANAGER_H

#include <QObject>

class ViewerThemeManager : public QObject {
    Q_OBJECT
    ViewerThemeManager(QObject* parent = 0);
public:
    enum AppTheme {
        Dark,
        Light,
    };

    static ViewerThemeManager* instance();
signals:
    void viewerThemeChanged(AppTheme theme);
public slots:
    AppTheme getCurrentTheme();
    void setCurrentTheme(AppTheme theme);

private:
    static ViewerThemeManager* m_viewerTheme;
    AppTheme m_currentTheme = AppTheme::Light;
};
#endif // VIEWERTHEMEMANAGER_H
