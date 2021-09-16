/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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
#include "imagedataservice.h"

#include <QMetaType>
#include <QDirIterator>
#include <QStandardPaths>
#include <QImageReader>

#include "unionimage/pluginbaseutils.h"
#include "unionimage/unionimage.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "commonservice.h"

ImageDataService *ImageDataService::s_ImageDataService = nullptr;

ImageDataService *ImageDataService::instance(QObject *parent)
{
    Q_UNUSED(parent);
    if (!s_ImageDataService) {
        s_ImageDataService = new ImageDataService();
    }
    return s_ImageDataService;
}

ImageDataService::~ImageDataService()
{
}

bool ImageDataService::add(const QStringList &paths)
{
    QMutexLocker locker(&m_imgDataMutex);
//    m_requestQueue.clear();
    for (int i = 0; i < paths.size(); i++) {
        if (!m_AllImageMap.contains(paths.at(i))) {
            m_requestQueue.append(paths.at(i));
        }
    }
    return true;
}

bool ImageDataService::add(const QString &path)
{
    QMutexLocker locker(&m_imgDataMutex);
    if (!path.isEmpty()) {
        if (!m_AllImageMap.contains(path)) {
            m_requestQueue.append(path);
        }
    }
    return true;
}

QString ImageDataService::pop()
{
    QMutexLocker locker(&m_imgDataMutex);
    if (m_requestQueue.empty())
        return QString();
    QString res = m_requestQueue.first();
    m_requestQueue.pop_front();
    return res;
}

bool ImageDataService::isRequestQueueEmpty()
{
    QMutexLocker locker(&m_imgDataMutex);
    return m_requestQueue.isEmpty();
}

int ImageDataService::getCount()
{
    return m_AllImageMap.count();
}
#include <QDebug>
bool ImageDataService::readThumbnailByPaths(QString thumbnailPath, QStringList files, bool remake)
{

    qDebug() << "------------files.size = " << files.size();
    bool empty = isRequestQueueEmpty();

    if (empty) {

        ImageDataService::instance()->add(files);
        int needCoreCounts = static_cast<int>(std::thread::hardware_concurrency());
        needCoreCounts = needCoreCounts / 2;
        if (files.size() < needCoreCounts) {
            needCoreCounts = files.size();
        }
        if (needCoreCounts < 1)
            needCoreCounts = 1;
        QList<QThread *> threads;
        for (int i = 0; i < needCoreCounts; i++) {
            readThumbnailThread *thread = new readThumbnailThread;
            thread->m_thumbnailPath = thumbnailPath;
            thread->m_remake = remake;
            thread->start();
            threads.append(thread);
        }
//        for (auto thread : threads) {
//            thread->wait();
//            thread->deleteLater();
//        }
    } else {
        ImageDataService::instance()->add(files);
    }
    return true;
}

#include "imageengine.h"
void ImageDataService::addImage(const QString &path, const QImage &image)
{
    QMutexLocker locker(&m_imgDataMutex);
    m_AllImageMap[path] = image;
    qDebug() << "------------m_requestQueue.size = " << m_requestQueue.size();
    qDebug() << "------------m_AllImageMap.size = " << m_AllImageMap.size();

//    emit ImageEngine::instance()->sigOneImgReady(path, info);

//    if (!m_AllImageMap.contains(path)) {
//        m_AllImageMap[path] = image;
//        while (m_AllImageMap.size() > 1000) {
//            //保证缓存占用，始终只占用1000张缩略图缓存
//            QString res = m_imageKey.first();
//            m_imageKey.pop_front();
//            m_AllImageMap.remove(res);
//        }
//    }
}

void ImageDataService::addMovieDurationStr(const QString &path, const QString &durationStr)
{
    QMutexLocker locker(&m_imgDataMutex);
    m_movieDurationStrMap[path] = durationStr;
}

QString ImageDataService::getMovieDurationStrByPath(const QString &path)
{
    QMutexLocker locker(&m_imgDataMutex);
    return m_movieDurationStrMap.contains(path) ? m_movieDurationStrMap[path] : QString() ;
}

void ImageDataService::setAllDataKeys(const QStringList &paths, bool single)
{
    Q_UNUSED(paths);
    Q_UNUSED(single);
}

void ImageDataService::setVisualIndex(int row)
{
    QMutexLocker locker(&m_imgDataMutex);
    m_visualIndex = row;
}

int ImageDataService::getVisualIndex()
{
    QMutexLocker locker(&m_imgDataMutex);
    return m_visualIndex;
}

QImage ImageDataService::getThumnailImageByPath(const QString &path)
{
    QMutexLocker locker(&m_imgDataMutex);
    return m_AllImageMap.contains(path) ? m_AllImageMap[path] : QImage();
}

bool ImageDataService::imageIsLoaded(const QString &path)
{
    QMutexLocker locker(&m_imgDataMutex);
    return m_AllImageMap.contains(path);
}

ImageDataService::ImageDataService(QObject *parent)
{
    Q_UNUSED(parent);
}

//缩略图读取线程
readThumbnailThread::readThumbnailThread(QObject *parent): QThread(parent)
{
}

readThumbnailThread::~readThumbnailThread()
{

}

void readThumbnailThread::readThumbnail(QString path)
{
    if (!QFileInfo(path).exists()) {
        return;
    }
    //新增,增加缓存
    imageViewerSpace::ItemInfo itemInfo;

    itemInfo.path = path;

    //获取路径类型
    itemInfo.pathType = getPathType(path);

    //获取原图分辨率
    QImageReader imagreader(path);
    itemInfo.imgOriginalWidth = imagreader.size().width();
    itemInfo.imgOriginalHeight = imagreader.size().height();

    using namespace UnionImage_NameSpace;
    QImage tImg;
    QString srcPath = path;
    //缩略图保存路径
    QString savePath = m_thumbnailPath + path;
    //保存为jpg格式
    savePath = savePath.mid(0, savePath.lastIndexOf('.')) + ImageEngine::instance()->makeMD5(savePath) + ".png";
    QFileInfo file(savePath);
    //缩略图已存在，执行下一个路径,如果读取的尺寸为错误,也需要重新读取
    if (!m_remake && file.exists() && itemInfo.imgOriginalWidth > 0 && itemInfo.imgOriginalHeight > 0) {
        tImg = QImage(savePath);
        itemInfo.image = tImg;
        //获取图片类型
        itemInfo.imageType = getImageType(path);
        CommonService::instance()->slotSetImgInfoByPath(path, itemInfo);
        return;
    }
    QString errMsg;
    QSize readSize;

    if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
        qDebug() << errMsg;
        return;
    }
    //读取图片,给长宽重新赋值
    itemInfo.imgOriginalWidth = tImg.width();
    itemInfo.imgOriginalHeight = tImg.height();

    if (0 != tImg.height() && 0 != tImg.width() && (tImg.height() / tImg.width()) < 10 && (tImg.width() / tImg.height()) < 10) {
        bool cache_exist = false;
        if (tImg.height() != 200 && tImg.width() != 200) {
            if (tImg.height() >= tImg.width()) {
                cache_exist = true;
                tImg = tImg.scaledToWidth(200,  Qt::SmoothTransformation);
            } else if (tImg.height() <= tImg.width()) {
                cache_exist = true;
                tImg = tImg.scaledToHeight(200,  Qt::SmoothTransformation);
            }
        }
        if (!cache_exist) {
            if (static_cast<float>(tImg.height()) / static_cast<float>(tImg.width()) > 3) {
                tImg = tImg.scaledToWidth(200,  Qt::SmoothTransformation);
            } else {
                tImg = tImg.scaledToHeight(200,  Qt::SmoothTransformation);
            }
        }
    }
    //创建路径
    pluginUtils::base::mkMutiDir(savePath.mid(0, savePath.lastIndexOf('/')));
    if (tImg.save(savePath)) {
        itemInfo.image = tImg;

    }
    if (itemInfo.image.isNull()) {
        itemInfo.imageType = imageViewerSpace::ImageTypeDamaged;
    } else {
        //获取图片类型
        itemInfo.imageType = getImageType(path);
    }
    CommonService::instance()->slotSetImgInfoByPath(path, itemInfo);
//    ImageDataService::instance()->addImage(path, tImg);
}

void readThumbnailThread::setQuit(bool quit)
{
    m_quit = quit;
}

imageViewerSpace::ImageType readThumbnailThread::getImageType(const QString &imagepath)
{
    return UnionImage_NameSpace::getImageType(imagepath);
}

imageViewerSpace::PathType readThumbnailThread::getPathType(const QString &imagepath)
{
    return UnionImage_NameSpace::getPathType(imagepath);
}

void readThumbnailThread::run()
{
    while (!ImageDataService::instance()->isRequestQueueEmpty()) {
        if (m_quit) {
            break;
        }
        QString res = ImageDataService::instance()->pop();
        if (!res.isEmpty()) {
            readThumbnail(res);
        }
    }
    emit ImageDataService::instance()->sigeUpdateListview();
    this->deleteLater();
}
