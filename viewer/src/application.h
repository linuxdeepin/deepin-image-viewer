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

#define protected public
#include <DApplication>
#undef protected
#include <QThread>
#include <QReadWriteLock>
#include <QMutex>
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
#define dApp (Application::getinstance())
/*lmh0728缩略图分辨率IMAGE_HEIGHT_DEFAULT*/
#define IMAGE_HEIGHT_DEFAULT    300

DWIDGET_USE_NAMESPACE

class ImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit ImageLoader(Application *parent, QStringList pathlist, QString path);

//    void addImageLoader(QStringList pathlist);

    /**
     * @brief updateImageLoader 更新缩略图，如果有旋转并旋转
     * @param pathlist          更新的图片路径
     * @param bDirection        旋转方向
     */
    void updateImageLoader(QStringList pathlist, bool bDirection,int rotateangle = 90);

    /**
     * @brief loadInterface 缩略图加载接口
     * @param strPath       加载的图片路径
     */
    void loadInterface(QString strPath);

    mutable QReadWriteLock m_readlock;
    mutable QReadWriteLock m_writelock;
    mutable QReadWriteLock m_flagLock;
public slots:

    /**
     * @brief startLoading  开始加载缩略图
     */
    void startLoading();

    /**
     * @brief stopThread    结束线程
     */
    void stopThread();


signals:
    /**
     * @brief sigFinishiLoad    缩略图加载完成信号
     * @param mapPath           加载完成的缩略图路径
     */
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

class Application : public QObject
{
    Q_OBJECT

public:
    ~Application();
    static Application *instance(int& argc, char **argv);
    static Application *getinstance();
    /**
     * @brief getRwLock 获取线程安全读写锁，在外部读取map时使用
     * @return          读写锁
     */
    inline QMutex &getRwLock()
    {
        return m_rwLock;
    }

    //20210111判断是否是平板模式
    bool isPanelDev()
    {
        return m_bIsPanel;
    }
    ConfigSetter *setter = nullptr;
    //    DBManager *dbM = nullptr;
    //    Exporter *exporter = nullptr;
    //    Importer *importer = nullptr;
    SignalManager *signalM = nullptr;
    WallpaperSetter *wpSetter = nullptr;
    ViewerThemeManager *viewerTheme = nullptr;

    QMap<QString, QPixmap> m_imagemap;
    QMap<QString, QRect> m_rectmap;
    ImageLoader *m_imageloader;

    QThread *m_LoadThread;
    bool m_firstLoad = true;
    DApplication* m_app;
    /*lmh0806儒码优化*/
    int  m_timer=0;
    bool m_bMove = false;
    bool m_bIsPanel=false;

    bool eventFilter(QObject *obj, QEvent *event);

signals:
    /**
     * @brief sigMouseRelease  全局线程释放事件
     * 2020/09/14 lmh
     */
    void sigMouseRelease();

    /**
     * @brief sigstartLoad  加载线程启动信号
     */
    void sigstartLoad();

    /**
     * @brief sigFinishLoad 缩略图加载完成信号
     * @param mapPath
     */
    void sigFinishLoad(QString mapPath);

    /**
     * @brief endThread 结束线程信号
     */
    void endThread();

    /**
     * @brief endApplication    结束程序的时候触发此信号
     */
    void endApplication();

    /**
     * @brief sigFinishiLoad    图片加载完成发送信号
     * @param mapPath           加载完成的图片路径
     */
    void sigFinishiLoad(QString mapPath);

    /**
     * @brief dynamicLoadFinished   动态加载完成,通知vipine可以允许进行删除操作了
     */
    void dynamicLoadFinished();

public slots:
    /**
     * @brief finishLoadSlot    缩略图加载完成信号
     * @param mapPath           加载完成的图片路径
     */
    void finishLoadSlot(QString mapPath);

    /**
     * @brief loadPixThread 开启动态加载图片线程
     * @param paths         动态加载的图片路径
     */
    void loadPixThread(QStringList paths);

    /**
     * @brief loadInterface 缩略图加载接口
     * @param strPath       加载的图片路径
     */
    void loadInterface(QString strPath);


    void quitApp();
private:
    /**
     * @brief initChildren  初始化子类对象
     */
    void initChildren();

    /**
     * @brief initI18n  初始化系统编码
     */
    void initI18n();

private:
    //读写锁
    QMutex m_rwLock;
    QStringList m_loadPaths;
    //线程结束标志位
    volatile bool m_bThreadExit = false;
    static Application *m_signalapp;
    Application(int &argc, char **argv);
};

#endif  // APPLICATION_H_
