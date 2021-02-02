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
#ifndef WALLPAPERSETTER_H
#define WALLPAPERSETTER_H

#include <QObject>

class WallpaperSetter : public QObject
{
    Q_OBJECT
public:
    static WallpaperSetter * instance();

    void setWallpaper(const QString &path);

    void setWallpaper(QImage img);
private:
    enum DE {
        Deepin,
        GNOME,
        GNOMEShell,
        KDE,
        LXDE,
        Xfce,
        MATE,
        OthersDE
    };

    void setGNOMEWallpaper(const QString &path);
    void setKDEWallpaper(const QString &path);
    void setLXDEWallpaper(const QString &path);
    void setXfaceWallpaper(const QString &path);

    DE getDE();
    bool testDE(const QString &app);

    explicit WallpaperSetter(QObject *parent = 0);

private:
    static WallpaperSetter *m_setter;
};

#endif // WALLPAPERSETTER_H
