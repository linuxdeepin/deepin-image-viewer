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
#ifndef ImgOperate_H
#define ImgOperate_H

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include <QImage>

class ImgOperate : public QObject
{
    Q_OBJECT
public:
    explicit ImgOperate(QObject *parent = nullptr);
    ~ImgOperate();

public slots:
    void slotMakeImgThumbnail(QString thumbnailSavePath, QStringList paths);

signals:
    void sigOneImgReady(QString imagepath, QImage image);

private:

};

#endif // ImgOperate_H
