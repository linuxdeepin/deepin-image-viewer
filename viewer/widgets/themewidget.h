#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include "controller/viewerthememanager.h"

#include <QWidget>
#include <QScrollArea>
#include <QFile>

class ThemeWidget : public QWidget {
    Q_OBJECT
public:
    ThemeWidget(const QString &darkFile, const QString &lightFile,
                QWidget* parent = 0);
    ~ThemeWidget();

public slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    QString m_darkStyle;
    QString m_lightStyle;
};

//TODO: if a widget Multiple Inheritance from ThemeWidget and
//      QScrollArea. warning(QWidget is an ambiguous base).
class ThemeScrollArea : public QScrollArea {
    Q_OBJECT
public:
    ThemeScrollArea(const QString &darkFile, const QString &lightFile,
                QWidget* parent = 0);
    ~ThemeScrollArea();

public slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    QString m_darkStyle;
    QString m_lightStyle;
};
#endif // THEMEWIDGET_H
