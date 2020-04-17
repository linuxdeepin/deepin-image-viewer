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
#include <QReadWriteLock>
#include <QList>
class Application;
class ConfigSetter;
class DatabaseManager;
class DBManager;
class Exporter;
class Importer;
class SignalManager;
class WallpaperSetter;
class ViewerThemeManager;
class QCloseEvent;
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
    void updateImageLoader(QStringList pathlist, bool bDirection);
    //add by heyi
    void loadInterface(QString strPath);
    mutable QReadWriteLock m_readlock;
    mutable QReadWriteLock m_writelock;
public slots:
    void startLoading();

    //add by heyi 结束线程
    void stopThread();

signals:
    void sigFinishiLoad(QString mapPath);

private:
    Application *m_parent;
    QStringList m_pathlist;
    QString m_path;
    //add by heyi
    volatile bool m_bFlag;
    QList<QString> listLoad1;
    QList<QString> listLoad2;
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
    //add by heyi
    void endThread();
    //结束程序的时候触发此信号
    void endApplication();

public slots:
    void finishLoadSlot(QString mapPath);

private:
    void initChildren();
    void initI18n();
};

#endif  // APPLICATION_H_
