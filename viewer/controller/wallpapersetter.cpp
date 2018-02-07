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
#include "wallpapersetter.h"
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QDBusInterface>

#include <unistd.h>

WallpaperSetter *WallpaperSetter::m_setter = nullptr;
WallpaperSetter *WallpaperSetter::instance()
{
    if (! m_setter) {
        m_setter = new WallpaperSetter();
    }

    return m_setter;
}

WallpaperSetter::WallpaperSetter(QObject *parent) : QObject(parent)
{

}

void WallpaperSetter::setWallpaper(const QString &path)
{
    // gsettings unsupported unicode character
    const QString tmpImg = QString("/tmp/DIVIMG.%1").arg(QFileInfo(path).suffix());
    QFile(path).copy(tmpImg);

#ifdef FLATPAK_SUPPORT
    // gdbus call -e -d com.deepin.daemon.Appearance -o /com/deepin/daemon/Appearance -m com.deepin.daemon.Appearance.Set background /home/test/test.png
    qDebug() << "SettingWallpaper: " << "flatpak" << path;
    QDBusInterface interface("com.deepin.daemon.Appearance",
                                 "/com/deepin/daemon/Appearance",
                                 "com.deepin.daemon.Appearance");
    if (interface.isValid()) {
        QDBusMessage reply = interface.call("Set", "background", path);
        qDebug() << "SettingWallpaper: replay" << reply.errorMessage();
    } else {
        qWarning() << "SettingWallpaper failed" << interface.lastError();
    }
#else
    DE de = getDE();
    qDebug() << "SettingWallpaper: " << de << path;
    switch (de) {
    case Deepin:
        setDeepinWallpaper(tmpImg);
        break;
    case KDE:
        setKDEWallpaper(tmpImg);
        break;
    case GNOME:
        setGNOMEWallpaper(tmpImg);
        break;
    case LXDE:
        setLXDEWallpaper(tmpImg);
        break;
    case Xfce:
        setXfaceWallpaper(tmpImg);
        break;
    default:
        setGNOMEShellWallpaper(tmpImg);
    }
#endif


    // Remove the tmp file
    QTimer *t = new QTimer(this);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, this, [t, tmpImg] {
        QFile(tmpImg).remove();
        t->deleteLater();
    });
    t->start(1000);
}

void WallpaperSetter::setDeepinWallpaper(const QString &path)
{
    QString comm = QString("gsettings set "
                           "com.deepin.wrap.gnome.desktop.background picture-uri %1").arg(path);
    QProcess::execute(comm);
}

void WallpaperSetter::setGNOMEWallpaper(const QString &path)
{
    QString comm = QString("gconftool-2 --type=string --set "
                           "/desktop/gnome/background/picture_filename %1").arg(path);
    QProcess::execute(comm);
}

void WallpaperSetter::setGNOMEShellWallpaper(const QString &path)
{
    QString comm = QString("gsettings set "
                           "org.gnome.desktop.background picture-uri %1").arg(path);
    QProcess::execute(comm);
}

void WallpaperSetter::setKDEWallpaper(const QString &path)
{
    QString comm = QString("dbus-send --session "
                           "--dest=org.new_wallpaper.Plasmoid --type=method_call "
                           "/org/new_wallpaper/Plasmoid/0 org.new_wallpaper.Plasmoid.SetWallpaper "
                           "string:%1").arg(path);
    QProcess::execute(comm);
}

void WallpaperSetter::setLXDEWallpaper(const QString &path)
{
    QString comm = QString("pcmanfm --set-wallpaper %1").arg(path);
    QProcess::execute(comm);
}

void WallpaperSetter::setXfaceWallpaper(const QString &path)
{
    QString comm = QString("xfconf-query -c xfce4-desktop -p "
                           "/backdrop/screen0/monitor0/image-path -s %1").arg(path);
    QProcess::execute(comm);
}

WallpaperSetter::DE WallpaperSetter::getDE()
{
    if (testDE("startdde")) {
        return Deepin;
    } else if (testDE("plasma-desktop")) {
        return KDE;
    } else if (testDE("gnome-panel") && qgetenv("DESKTOP_SESSION") == "gnome") {
        return GNOME;
    } else if (testDE("xfce4-panel")) {
        return Xfce;
    } else if (testDE("mate-panel")) {
        return MATE;
    } else if (testDE("lxpanel")) {
        return LXDE;
    } else {
        return OthersDE;
    }
}

bool WallpaperSetter::testDE(const QString &app)
{
    bool v;
    QProcess p;
    p.start(QString("ps -A"));
    while (p.waitForReadyRead(3000)) {
        v = QString(p.readAllStandardOutput()).contains(app);
        p.kill();
    }

    return v;
}
