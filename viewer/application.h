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
#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <DApplication>
#include <QThread>
class Application;
class ConfigSetter;
class DatabaseManager;
class DBManager;
class Exporter;
class Importer;
class SignalManager;
class WallpaperSetter;
class ViewerThemeManager;
#if defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<Application*>(QCoreApplication::instance()))

DWIDGET_USE_NAMESPACE

class ImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit ImageLoader(Application *parent, QStringList pathlist, QString path);

    void addImageLoader(QStringList pathlist);
    void updateImageLoader(QStringList pathlist);

public slots:
    void startLoading();

signals:
    void sigFinishiLoad(QString mapPath);

private:
    Application *m_parent;
    QStringList m_pathlist;
    QString m_path;
};

class Application : public DApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();

    ConfigSetter *setter = nullptr;
    //    DBManager *dbM = nullptr;
    //    Exporter *exporter = nullptr;
    //    Importer *importer = nullptr;
    SignalManager *signalM = nullptr;
    WallpaperSetter *wpSetter = nullptr;
    ViewerThemeManager *viewerTheme = nullptr;

    QMap<QString, QPixmap> m_imagemap;
    ImageLoader *m_imageloader;

    QThread *m_LoadThread;
signals:
    void sigstartLoad();
    void sigFinishLoad(QString mapPath);

public slots:
    void finishLoadSlot(QString mapPath);

private:
    void initChildren();
    void initI18n();
};

#endif  // APPLICATION_H_
