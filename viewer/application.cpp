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

namespace {

}  // namespace

#define IMAGE_HEIGHT_DEFAULT    100

ImageLoader::ImageLoader(Application *parent, QStringList pathlist, QString path)
{
    m_parent = parent;
    m_pathlist = pathlist;
    m_path = path;
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

    QStringList list;
    for (int i = 0; i < 25; i++) {
        if ((array - i) > -1)
            list.append(m_pathlist.at(array - i));
        if ((array + i) < count)
            list.append(m_pathlist.at(array + i));
    }

    for (QString path : list) {
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

        emit sigFinishiLoad(path);
    }

    num = 0;
    for (QString path : m_pathlist) {
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

        num++;
//        if (10 > num)
//        {
//            emit sigFinishiLoad();
//        }
//        else if (50 > num)
//        {
//            if (0 == num%3)
//            {
//                emit sigFinishiLoad();
//            }
//        }
//        else
//        {
//            if (0 == num%100)
//            {
//                emit sigFinishiLoad();
//            }
//        }
        emit sigFinishiLoad(path);
    }

    QString map = "";
    emit sigFinishiLoad(map);

    gettimeofday(&tv, NULL);
    ms = (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    qDebug() << "startLoading end time: " << ms;
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

void ImageLoader::updateImageLoader(QStringList pathlist)
{
    for (QString path : pathlist) {
        QPixmap pixmap(path);

        m_parent->m_imagemap[path] = pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation);
    }
}

void Application::finishLoadSlot(QString mapPath)
{
    qDebug() << "finishLoadSlot";
    emit sigFinishLoad(mapPath);
}

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
    initI18n();
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
        emit sigstartLoad();
    });

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
