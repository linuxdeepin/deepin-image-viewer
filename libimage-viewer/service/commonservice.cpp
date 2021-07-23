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
#include "commonservice.h"

#include <QImage>

CommonService *CommonService::m_commonService = nullptr;
CommonService *CommonService::instance()
{
    if (m_commonService == nullptr) {
        m_commonService = new CommonService;
    }

    return m_commonService;
}

void CommonService::setImgViewerType(ImgViewerType type)
{
    m_imgViewerType = type;
}

ImgViewerType CommonService::getImgViewerType()
{
    return m_imgViewerType;
}

void CommonService::setImgSavePath(QString path)
{
    m_imgSavePath = path;
}

QString CommonService::getImgSavePath()
{
    return m_imgSavePath;
}

void CommonService::setImgByPath(QString path, QImage image)
{
    m_allImageMap[path] = image;
}

QImage CommonService::getImgByPath(QString path)
{
    return m_allImageMap[path];
}

CommonService::CommonService(QObject *parent) : QObject(parent)
{

}
