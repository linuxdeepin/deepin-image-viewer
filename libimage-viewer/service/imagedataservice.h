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
#ifndef IMAGEDATASERVICE_H
#define IMAGEDATASERVICE_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QMutex>
#include <QQueue>
#include <QThread>

#include "image-viewer_global.h"

class readThumbnailThread;
class ImageDataService: public QObject
{
    Q_OBJECT
public:
    static ImageDataService *instance(QObject *parent = nullptr);
    explicit ImageDataService(QObject *parent = nullptr);
    ~ImageDataService();

    bool add(const QStringList &paths);
    bool add(const QString &path);
    QString pop();
    bool isRequestQueueEmpty();
    //获取全部图片数量
    int getCount();

    //读取缩略图到缓存map
    bool readThumbnailByPaths(QString thumbnailPath, QStringList files);
//    bool readThumbnailByPath(QString file);

    void addImage(const QString &path, const QImage &image);
    QImage getThumnailImageByPath(const QString &path);
    bool imageIsLoaded(const QString &path);

    void addMovieDurationStr(const QString &path, const QString &durationStr);
    QString getMovieDurationStrByPath(const QString &path);

    //设置当前窗口所有数据
    void setAllDataKeys(const QStringList &paths, bool single = false);

    //
    void setVisualIndex(int row);
    int getVisualIndex();
private slots:
signals:
    void sigeUpdateListview();
public:
private:
    static ImageDataService *s_ImageDataService;
    QMutex m_queuqMutex;
    QList<QString> m_requestQueue;

    //图片数据锁
    QMutex m_imgDataMutex;
    QMap<QString, QImage> m_AllImageMap;
    QMap<QString, QString> m_movieDurationStrMap;
    QQueue<QString> m_imageKeys;
    int m_visualIndex = 0;//用户查找视图中的model index
};


//缩略图读取线程
class readThumbnailThread : public QThread
{
    Q_OBJECT
public:
    readThumbnailThread(QObject *parent = nullptr);
    ~readThumbnailThread() override;
    void readThumbnail(QString m_path);
    void setQuit(bool quit);
    QString m_thumbnailPath = "";

    //判断图片类型
    imageViewerSpace::ImageType getImageType(const QString &imagepath);
    //判断路径类型
    imageViewerSpace::PathType getPathType(const QString &imagepath);
protected:
    void run() override;
private:
    bool m_quit = false;

};
#endif // IMAGEDATASERVICE_H
