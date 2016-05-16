#ifndef WALLPAPERSETTER_H
#define WALLPAPERSETTER_H

#include <QObject>

class WallpaperSetter : public QObject
{
    Q_OBJECT
public:
    static WallpaperSetter * instance();
    void setWallpaper(const QString &path);

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

    void setDeepinWallpaper(const QString &path);
    void setGNOMEWallpaper(const QString &path);
    void setGNOMEShellWallpaper(const QString &path);
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
