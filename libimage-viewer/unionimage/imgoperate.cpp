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
#include <QImageReader>
#include <QMimeDatabase>

#include "unionimage.h"
#include "pluginbaseutils.h"
#include "imageengine.h"
#include "imageutils.h"


ImgOperate::ImgOperate(QObject *parent)
{
    Q_UNUSED(parent);
}

ImgOperate::~ImgOperate()
{

}

void ImgOperate::slotMakeImgThumbnail(QString thumbnailSavePath, QStringList paths, int makeCount, bool remake)
{
    QString path;
    QImage tImg;
    imageViewerSpace::ItemInfo itemInfo;
    for (int i = 0; i < paths.size(); i++) {
        //达到制作数量则停止
        if (i == makeCount) {
            break;
        }
        path = paths.at(i);
        itemInfo.path = path;

        //获取路径类型
        itemInfo.pathType = getPathType(path);

        //获取原图分辨率
        QImageReader imagreader(path);

        if (imagreader.size().width() == -1 || imagreader.size().height() == -1) {
            //Qt读取图片尺寸失败，转向使用FreeImage，注意：读meta data有几率读取失败，采用这个函数读是最保险的，这个位置我们将确保读到所有支持图片的尺寸大小
            QString errMsg;
            if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
                qDebug() << errMsg;
                continue;
            }
            itemInfo.imgOriginalWidth = tImg.size().width();
            itemInfo.imgOriginalHeight = tImg.size().height();
        } else {
            itemInfo.imgOriginalWidth = imagreader.size().width();
            itemInfo.imgOriginalHeight = imagreader.size().height();
        }

        //缩略图保存路径
        QString savePath = thumbnailSavePath + path;
        //保存为jpg格式
        savePath = savePath.mid(0, savePath.lastIndexOf('.')) + ImageEngine::instance()->makeMD5(savePath) + ".png";
        QFileInfo file(savePath);
        //缩略图已存在，执行下一个路径
        if (file.exists() && !remake) {
            tImg = QImage(savePath);
            itemInfo.image = tImg;
            //获取图片类型
            itemInfo.imageType = getImageType(path);
            emit sigOneImgReady(path, itemInfo);
            continue;
        }

        if (tImg.isNull()) { //如果前面用过FreeImage读图片尺寸，那么这里不需要重复读取图片
            QString errMsg;
            if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
                qDebug() << errMsg;
                continue;
            }
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
            itemInfo.image = tImg;

        }
        if (itemInfo.image.isNull()) {
            itemInfo.imageType = imageViewerSpace::ImageTypeDamaged;
        } else {
            //获取图片类型
            itemInfo.imageType = getImageType(path);
        }
        emit sigOneImgReady(path, itemInfo);
    }
}

imageViewerSpace::ImageType ImgOperate::getImageType(const QString &imagepath)
{
    return UnionImage_NameSpace::getImageType(imagepath);
}

imageViewerSpace::PathType ImgOperate::getPathType(const QString &imagepath)
{
    //判断文件路径来自于哪里
    imageViewerSpace::PathType type = imageViewerSpace::PathType::PathTypeLOCAL;
    if (imagepath.indexOf("smb-share:server=") != -1) {
        type = imageViewerSpace::PathTypeSMB;
    } else if (imagepath.indexOf("mtp:host=") != -1) {
        type = imageViewerSpace::PathTypeMTP;
    } else if (imagepath.indexOf("gphoto2:host=") != -1) {
        type = imageViewerSpace::PathTypePTP;
    } else if (imagepath.indexOf("gphoto2:host=Apple") != -1) {
        type = imageViewerSpace::PathTypeAPPLE;
    } else if (utils::image::isVaultFile(imagepath)) {
        type = imageViewerSpace::PathTypeSAFEBOX;
    } else if (imagepath.contains(QDir::homePath() + "/.local/share/Trash")) {
        type = imageViewerSpace::PathTypeRECYCLEBIN;
    }
    //todo
    return type;
}
