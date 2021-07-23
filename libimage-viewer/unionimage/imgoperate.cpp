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
#include "imgoperate.h"

#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QStandardPaths>
#include <QDirIterator>
#include <QThread>
#include <QImage>

#include "unionimage.h"
#include "pluginbaseutils.h"
#include "imageengine.h"

ImgOperate::ImgOperate(QObject *parent)
{
    Q_UNUSED(parent);
}

ImgOperate::~ImgOperate()
{

}

void ImgOperate::slotMakeImgThumbnail(QString thumbnailSavePath, QStringList paths, int makeCount)
{
    qDebug() << "---" << __FUNCTION__ << "---thumbnailSavePath = " << thumbnailSavePath;
    qDebug() << "---" << __FUNCTION__ << "---" << QThread::currentThread();
    QString path;
    for (int i = 0; i < paths.size(); i++) {
        //达到制作数量则停止
        if (i == makeCount) {
            break;
        }
        path = paths.at(i);
        //路径为空，继续执行下一个路径
        if (path.isEmpty()) {
            continue;
        }
        QImage tImg;
        //缩略图路径
        QString savePath = thumbnailSavePath + path;

        //保存为jpg格式
        savePath = savePath.mid(0, savePath.lastIndexOf('.')) + ImageEngine::instance()->makeMD5(savePath) + ".png";

        QFileInfo file(savePath);
        //缩略图已存在，执行下一个路径
        if (file.exists()) {
            continue;
        }
        QString errMsg;
        QSize readSize;
//        if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, readSize, errMsg)) {
//            qDebug() << errMsg;
//            continue;
//        }
        if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
            qDebug() << errMsg;
            continue;
        }

        if (0 != tImg.height() && 0 != tImg.width() && (tImg.height() / tImg.width()) < 10 && (tImg.width() / tImg.height()) < 10) {
            bool cache_exist = false;
            if (tImg.height() != 200 && tImg.width() != 200) {
                if (tImg.height() >= tImg.width()) {
                    cache_exist = true;
                    tImg = tImg.scaledToWidth(200,  Qt::FastTransformation);
                } else if (tImg.height() <= tImg.width()) {
                    cache_exist = true;
                    tImg = tImg.scaledToHeight(200,  Qt::FastTransformation);
                }
            }
            if (!cache_exist) {
                if (static_cast<float>(tImg.height()) / static_cast<float>(tImg.width()) > 3) {
                    tImg = tImg.scaledToWidth(200,  Qt::FastTransformation);
                } else {
                    tImg = tImg.scaledToHeight(200,  Qt::FastTransformation);
                }
            }
        }
        //创建路径
        pluginUtils::base::mkMutiDir(savePath.mid(0, savePath.lastIndexOf('/')));

        if (tImg.save(savePath)) {
            emit sigOneImgReady(path, tImg);
        }
    }
}
