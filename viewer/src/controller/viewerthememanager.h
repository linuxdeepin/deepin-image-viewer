/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef VIEWERTHEMEMANAGER_H
#define VIEWERTHEMEMANAGER_H

#include <QObject>

class ViewerThemeManager : public QObject {
    Q_OBJECT
    ViewerThemeManager(QObject* parent = nullptr);
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
