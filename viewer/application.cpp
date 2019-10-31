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

ImageLoader::ImageLoader(Application* parent, QStringList pathlist)
{
    m_parent = parent;
    m_pathlist = pathlist;
}

void ImageLoader::startLoading()
{
    struct timeval tv;
    long long ms;
    gettimeofday(&tv,NULL);
    ms = (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
    qDebug()<<"startLoading start time: "<<ms;


    for(QString path : m_pathlist)
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

        m_parent->m_imagemap.insert(path, pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation));
    }


    emit sigFinishiLoad();

    gettimeofday(&tv,NULL);
    ms = (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
    qDebug()<<"startLoading end time: "<<ms;
}

void ImageLoader::addImageLoader(QStringList pathlist)
{
    for(QString path : pathlist)
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

        m_parent->m_imagemap.insert(path, pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation));
    }
}

void ImageLoader::updateImageLoader(QStringList pathlist)
{
    for(QString path : pathlist)
    {
        QPixmap pixmap(path);

        m_parent->m_imagemap[path] = pixmap.scaledToHeight(IMAGE_HEIGHT_DEFAULT,  Qt::FastTransformation);
    }
}

void Application::finishLoadSlot()
{
    qDebug()<<"finishLoadSlot";
    emit sigFinishLoad();
}

Application::Application(int& argc, char** argv)
    : DApplication(argc, argv)
{
    initI18n();
    setOrganizationName("deepin");
    setApplicationName("deepin-image-viewer");
    setApplicationDisplayName(tr("Deepin Image Viewer"));
    setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    setApplicationDescription(QString("%1\n%2\n").arg(tr("深度看图是一款外观时尚,性能流畅的图片查看工具。")).arg(tr("深度看图是一款外观时尚,性能流畅的图片查看工具。")));

//    //save theme
//    DApplicationSettings saveTheme;

//    setApplicationVersion(DApplication::buildVersion("1.3"));
    setApplicationVersion(DApplication::buildVersion("20190828"));
    installEventFilter(new GlobalEventFilter());


    initChildren();


    connect(dApp->signalM, &SignalManager::Sendpathlist, this, [=](QStringList list){
        m_imageloader= new ImageLoader(this, list);
        m_LoadThread = new QThread();

        m_imageloader->moveToThread(m_LoadThread);
        m_LoadThread->start();

        connect(this, SIGNAL(sigstartLoad()), m_imageloader, SLOT(startLoading()));
        connect(m_imageloader, SIGNAL(sigFinishiLoad()), this, SLOT(finishLoadSlot()));
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
