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
#include "dbusclient.h"

Dbusclient::Dbusclient(QObject *parent)
    : QDBusAbstractInterface(staticInterfaceService(), staticInterfacePath(),
                             staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{

    // QDBusConnection::sessionBus().connect(staticInterfaceService(),staticInterfacePath(),staticInterfaceName(),"com.deepin.Draw",  "com.deepin.Draw",this,SLOT(propertyChanged(QDBusMessage)));

    connect(dApp->signalM, &SignalManager::sigDrawingBoard, this, &Dbusclient::openDrawingBoard);
}

Dbusclient::~Dbusclient()
{

}

//void Dbusclient::propertyChanged(const QDBusMessage &msg)
//{
//    Q_UNUSED(msg)
//}


void Dbusclient::openDrawingBoard(const QStringList &paths)
{
    QList<QString> list = paths;

    openFiles(list);
}

