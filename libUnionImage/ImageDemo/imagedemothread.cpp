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
#include "imagedemothread.h"

RotateSaveThread::RotateSaveThread()
{
    setAutoDelete(true);
}

void RotateSaveThread::setDatas(QHash<QString, RotateSaveRequest> requests_bar)
{
    for (RotateSaveRequest i : requests_bar) {
        m_requests.append(i);
    }
}

void RotateSaveThread::run()
{
    for (RotateSaveRequest i : m_requests) {
        QString errorMsg;
        if (!UnionImage_NameSpace::rotateImageFIleWithImage(static_cast<int>(i.angel), i.image, i.path, errorMsg)) {
            qDebug() << errorMsg;
            qDebug() << "Save error";
        } else {
            qDebug() << "Save Success";
        }
    }
    if (m_requests.empty()) {
        qDebug() << "No Pic and Run Thread";
    } else {
        qDebug() << "Save End";
    }

}

ImageDemoThreadControler::ImageDemoThreadControler()
{
    wait = new QTimer(this);
    rotateThreadPool.setMaxThreadCount(5);
    connect(wait, &QTimer::timeout, this, &ImageDemoThreadControler::startSave);
}

ImageDemoThreadControler::~ImageDemoThreadControler()
{
    rotateThreadPool.waitForDone();
}
void ImageDemoThreadControler::addRotateAndSave(RotateSaveRequest request, int time_gap)
{
    if (NoRepeatRequest.contains(request.path)) {
        NoRepeatRequest[request.path].angel += request.angel;
        emit updateRotate(static_cast<int>(NoRepeatRequest[request.path].angel));
    } else {
        NoRepeatRequest.insert(request.path, request);
    }
    wait->start(time_gap);
}

void ImageDemoThreadControler::startSave()
{
    emit updateRotate(0);
    RotateSaveThread *thread = new RotateSaveThread;
    thread->setDatas(NoRepeatRequest);
    NoRepeatRequest.clear();
    rotateThreadPool.start(thread);
    wait->stop();
}
