/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Deng jinhui<dengjinhui@uniontech.com>
*
* Maintainer: Deng jinhui <dengjinhui@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef IMAGEDEMOTHREAD_H
#define IMAGEDEMOTHREAD_H

#include <unionimage.h>
#include <QObject>
#include <QVector>
#include <QList>
#include <QHash>
#include <QRunnable>
#include <QThreadPool>
#include <QImage>
#include <QDebug>
#include <QTimer>

struct RotateSaveRequest {
    double angel;
    QImage image;
    QString path;
};

class RotateSaveThread : public QRunnable
{
public:
    RotateSaveThread();
    void setDatas(QHash<QString, RotateSaveRequest>   requests_bar);
protected:
    void run();
private:
    QVector<RotateSaveRequest> m_requests;
};

class ImageDemoThreadControler : public QObject
{
    Q_OBJECT
public:
    ImageDemoThreadControler();
    ~ImageDemoThreadControler();
signals:
    void updateRotate(int angel);
public slots:
    /**
     * @brief addRotateAndSave
     * @param request
     * @param time_gap
     * @author DJH
     * 添加旋转请求队列，并设置队列清空等待时间，等待时间结束后，会执行队列所有命令
     */
    void addRotateAndSave(RotateSaveRequest request, int time_gap);
    void startSave();

private:
    QTimer *wait;
    QThreadPool rotateThreadPool;
    //QList<RotateSaveThread *> rotateThreads;
    QHash<QString, RotateSaveRequest> NoRepeatRequest;
};

#endif // IMAGEDEMOTHREAD_H
