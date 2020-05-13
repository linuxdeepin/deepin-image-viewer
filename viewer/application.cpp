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
#include "application.h"

#include "controller/configsetter.h"
#include "controller/globaleventfilter.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "controller/viewerthememanager.h"
#include "utils/snifferimageformat.h"

#include <QDebug>
#include <QTranslator>
#include <DApplicationSettings>
#include <QIcon>
#include <QImageReader>
#include <sys/time.h>
#include <QFile>
#include <QImage>
#include <QQueue>

namespace {

}  // namespace

#define IMAGE_HEIGHT_DEFAULT    100

ImageLoader::ImageLoader(Application *parent, QStringList pathlist, QString path)
{
    m_parent = parent;
    m_pathlist = pathlist;
    m_path = path;
    m_bFlag = true; //heyi
}

void ImageLoader::startLoading()
{
    struct timeval tv;
    long long ms;
    gettimeofday(&tv, NULL);
    ms = (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    qDebug() << "startLoading start time: " << ms;

    int num = 0;
    int array = 0;
    int count = m_pathlist.size();

    for (QString path : m_pathlist) {
        num++;
        if (m_path == path) {
            array = num - 1;
            num = 0;
        }
    }

    //heyi test
    QStringList list;
    for (int i = 0; i < 25; i++) {
        if ((array - i) > -1) {
            list.append(m_pathlist.at(array - i));
        }
        if ((array + i) < count) {
            list.append(m_pathlist.at(array + i));
        }
    }

    //heyi test 开辟两个线程同时加载,重选中位置往两边加载
    for (int i = 0; i < array; i++) {
        listLoad1.append(m_pathlist.at(i));
    }

    for (int i = array; i < m_pathlist.size(); i++) {
        listLoad2.append(m_pathlist.at(i));
    }

    QThread *th1 = QThread::create([ = ]() {
        QString path;
        while (1) {
            path.clear();
            m_readlock.lockForWrite();
            if (listLoad1.isEmpty()) {
                m_readlock.unlock();
                break;
            }

            //判断线程标识
            m_flagLock.lockForRead();
            if (!m_bFlag) {
                m_flagLock.unlock();
                break;
            }

            m_flagLock.unlock();

            path = listLoad1.back();
            qDebug() << "线程1当前加载的图片：" << path;
            //加载完成之后删除该图片路径
            listLoad1.pop_back();
            m_readlock.unlock();
            loadInterface(path);
        }

        //帮助另一个线程加载，从左往右加载
        while (1) {
            path.clear();
            m_readlock.lockForWrite();
            if (listLoad2.isEmpty()) {
                m_readlock.unlock();
                break;
            }

            //判断线程标识
            m_flagLock.lockForRead();
            if (!m_bFlag) {
                m_flagLock.unlock();
                break;
            }

            m_flagLock.unlock();

            path = listLoad2.front();
            qDebug() << "线程1当前加载的图片：" << path;
            listLoad2.pop_front();
            m_readlock.unlock();
            loadInterface(path);
        }

        QThread::currentThread()->quit();
    });

    QThread *th2 = QThread::create([ = ]() {
        QString path;
        while (1) {
            path.clear();
            m_readlock.lockForWrite();
            if (listLoad2.isEmpty()) {
                m_readlock.unlock();
                break;
            }

            //判断线程标识
            m_flagLock.lockForRead();
            if (!m_bFlag) {
                m_flagLock.unlock();
                break;
            }

            m_flagLock.unlock();

            path = listLoad2.front();
            qDebug() << "线程2当前加载的图片：" << path;
            //加载完成之后删除该图片路径
            listLoad2.pop_front();
            m_readlock.unlock();
            loadInterface(path);
        }

        //帮助另一个线程加载，从右往左加载
        while (1) {
            path.clear();
            m_readlock.lockForWrite();
            if (listLoad1.isEmpty()) {
                m_readlock.unlock();
                break;
            }

            //判断线程标识
            m_flagLock.lockForRead();
            if (!m_bFlag) {
                m_flagLock.unlock();
                break;
            }

            m_flagLock.unlock();

            path = listLoad1.back();
            qDebug() << "线程2当前加载的图片：" << path;
            listLoad1.pop_back();
            m_readlock.unlock();
            loadInterface(path);
        }

        QThread::currentThread()->quit();
    });

    th1->start();
    th2->start();

    QString map = "";
    emit sigFinishiLoad(map);

    gettimeofday(&tv, NULL);
    ms = (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void ImageLoader::stopThread()
{
    m_flagLock.lockForWrite();
    m_bFlag = false;
    m_flagLock.unlock();
}

void ImageLoader::addImageLoader(QStringList pathlist)
{
    for (QString path : pathlist) {
        QImage tImg;

        QString format = DetectImageFormat(path);
        if (format.isEmpty()) {
            QImageReader reader(path);
            reader.setAutoTransform(true);
            if (reader.canRead()) {
                tImg = reader.read();
            }
        } else {
            QImageReader readerF(path, format.toLatin1());
            readerF.setAutoTransform(true);
            if (readerF.canRead()) {
                tImg = readerF.read();
            } else {
                qWarning() << "can't read image:" << readerF.errorString()
                           << format;

                tImg = QImage(path);
            }
        }
        QPixmap pixmap = QPixmap::fromImage(tImg);

        m_parent->m_imagemap.insert(path, pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation));
    }
}
//modify by heyi
void ImageLoader::updateImageLoader(QStringList pathlist, bool bDirection)
{
    for (QString path : pathlist) {
        dApp->getRwLock().lockForWrite();
        QPixmap pixmap = m_parent->m_imagemap[path];
        if (pixmap.isNull()) {
            QImage image(path);
            pixmap = QPixmap::fromImage(image);
        } else {
            QMatrix rotate;
            if (bDirection) {
                rotate.rotate(90);
            } else {
                rotate.rotate(-90);
            }

            pixmap = pixmap.transformed(rotate, Qt::FastTransformation);
        }

        //QImage image(path);
        //QPixmap pixmap = QPixmap::fromImage(image);
        m_parent->m_imagemap[path] = pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation);
        dApp->getRwLock().unlock();
    }
}

void ImageLoader::loadInterface(QString path)
{
    QImage tImg;
    QString format = DetectImageFormat(path);
    if (format.isEmpty()) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            tImg = reader.read();
        }
    } else {
        QImageReader readerF(path, format.toLatin1());
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            tImg = readerF.read();
        } else {
            qWarning() << "can't read image:" << readerF.errorString()
                       << format;

            tImg = QImage(path);
        }
    }

    QPixmap pixmap = QPixmap::fromImage(tImg);

    m_writelock.lockForWrite();
    m_parent->m_imagemap.insert(path, pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation));
    m_writelock.unlock();

    emit sigFinishiLoad(path);
}

void Application::finishLoadSlot(QString mapPath)
{
    qDebug() << "finishLoadSlot";
    emit sigFinishLoad(mapPath);
}

void Application::loadPixThread(QStringList paths)
{
    m_rwLock.lockForWrite();
    m_loadPaths = paths;
    m_rwLock.unlock();
    //开启线程进行后台加载图片
    QThread *th = QThread::create([ = ]() {
        m_rwLock.lockForRead();
        QStringList pathList = m_loadPaths;
        m_rwLock.unlock();
        foreach (QString var, pathList) {
            if (m_bThreadExit) {
                break;
            }

            loadInterface(var);
        }

        //发送动态加载完成信号
        emit dynamicLoadFinished();
        QThread::currentThread()->quit();
    });

    th->start();
}

void Application::loadInterface(QString path)
{
    QImage tImg;
    QString format = DetectImageFormat(path);
    if (format.isEmpty()) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            tImg = reader.read();
        }
    } else {
        QImageReader readerF(path, format.toLatin1());
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            tImg = readerF.read();
        } else {
            qWarning() << "can't read image:" << readerF.errorString()
                       << format;

            tImg = QImage(path);
        }
    }

    QPixmap pixmap = QPixmap::fromImage(tImg);
    m_rwLock.lockForWrite();
    m_imagemap.insert(path, pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation));
    m_rwLock.unlock();

    finishLoadSlot(path);
}

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
    initI18n();
    m_LoadThread = nullptr;
    setOrganizationName("deepin");
    setApplicationName("deepin-image-viewer");
    setApplicationDisplayName(tr("Image Viewer"));
    setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
//    setApplicationDescription(QString("%1\n%2\n").arg(tr("看图是⼀款外观时尚，性能流畅的图片查看工具。")).arg(tr("看图是⼀款外观时尚，性能流畅的图片查看工具。")));
//    setApplicationAcknowledgementPage("https://www.deepin.org/" "acknowledgments/deepin-image-viewer/");
    setApplicationDescription(tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));

//    //save theme
//    DApplicationSettings saveTheme;

//    setApplicationVersion(DApplication::buildVersion("1.3"));
    setApplicationVersion(DApplication::buildVersion("20190828"));
    installEventFilter(new GlobalEventFilter());


    initChildren();



    connect(dApp->signalM, &SignalManager::sendPathlist, this, [ = ](QStringList list, QString path) {
        m_imageloader = new ImageLoader(this, list, path);
        m_LoadThread = new QThread();

        m_imageloader->moveToThread(m_LoadThread);
        m_LoadThread->start();

        connect(this, SIGNAL(sigstartLoad()), m_imageloader, SLOT(startLoading()));
        connect(m_imageloader, SIGNAL(sigFinishiLoad(QString)), this, SLOT(finishLoadSlot(QString)));
        //heyi
        connect(this, SIGNAL(endThread()), m_imageloader, SLOT(stopThread()), Qt::QueuedConnection);
        emit sigstartLoad();
    });
}

Application::~Application()
{
    if (nullptr !=  m_LoadThread) {
        if (m_LoadThread->isRunning()) {
            //结束线程
            m_LoadThread->requestInterruption();
            m_bThreadExit = true;
            emit endThread();
            QThread::msleep(1000);
            m_LoadThread->quit();
        }
    }

    emit endApplication();
}

void Application::initChildren()
{
    viewerTheme = ViewerThemeManager::instance();
    setter = ConfigSetter::instance();
    signalM = SignalManager::instance();
    wpSetter = WallpaperSetter::instance();
}

void Application::initI18n()
{
    // install translators
//    QTranslator *translator = new QTranslator;
//    translator->load(APPSHAREDIR"/translations/deepin-image-viewer_"
//                     + QLocale::system().name() + ".qm");
//    installTranslator(translator);
    loadTranslator(QList<QLocale>() << QLocale::system());
}
