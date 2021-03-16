/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
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
#ifndef DBUSCLIENTDROW_H
#define DBUSCLIENTDROW_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QImage>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QtDBus>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <application.h>
#include "signalmanager.h"

class Dbusclient : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceService()
    { return "com.deepin.Draw"; }
    static inline const char *staticInterfacePath()
    { return "/com/deepin/Draw"; }
    static inline const char *staticInterfaceName()
    { return "com.deepin.Draw"; }

public:
    explicit Dbusclient(QObject *parent = nullptr);
    ~Dbusclient();



public Q_SLOTS:
//    void propertyChanged(const QDBusMessage &msg);
    /*
    * @bref:openFiles 通过路径打开图片文件
    * @param: filePaths 图片的路径
    * @return: QDBusPendingReply
    */
    inline QDBusPendingReply<> openFiles(const QList<QString> &filePaths)
    {
        QList<QVariant> argumentList;
        for (QString path : filePaths) {
            argumentList << QVariant::fromValue(path.toLocal8Bit());
        }
        return call(QStringLiteral("openFiles"), argumentList);
    }

    /*
    * @bref:openImages 通过QImage打开图片
    * @param: filePaths 图片
    * @return: QDBusPendingReply
    * @note: 建议不要一次打开多个图片大文件，会比较卡
    */
    inline QDBusPendingReply<> openImages(const QList<QImage> &images)
    {
        QList<QVariant> argumentList;
        for (QImage img : images) {
            QByteArray data;
            QBuffer buf(&data);
            if (img.save(&buf, "PNG")) {
                data = qCompress(data, 9);
                data = data.toBase64();
                argumentList << QVariant::fromValue(data);
            }
        }

        return call(QStringLiteral("openImages"), argumentList);
    }
    void openDrawingBoard(const QStringList &paths);


};

#endif // DBUSCLIENTDROW_H
