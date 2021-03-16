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
#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"
#include "accessibility/acobjectlist.h"
#include "config.h"

#include <QApplication>
#include <DLog>
#include <QTranslator>
#include <DApplicationSettings>

using namespace Dtk::Core;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

//    Application::loadDXcbPlugin();
    Application::instance(argc, argv);

    dApp->m_app->setAttribute(Qt::AA_ForceRasterWidgets);
    dApp->m_app->installEventFilter(dApp);
#ifdef INSTALLACCESSIBLEFACTORY
    QAccessible::installFactory(accessibleFactory);
#endif
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();
    //增加版本号
    qApp->setApplicationVersion(DApplication::buildVersion(VERSION));
    if (dApp->isPanelDev()) {
        //进程单例
        QString userpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QSharedMemory sharedMemory(userpath + QString("deepinimangeviewsingle"));
        //if (sharedMemory.isAttached()) {
        //   sharedMemory.detach();
        // }
        //1.申请QBuffer
        QBuffer buffer;
        //2.将buffer写入data流中
        QDataStream out(&buffer);
        //3.buffer读写操作  利用QBuffer将图片数据转化为char * 格式
        buffer.open(QBuffer::ReadWrite);
        //4.将时间写入QDataStream
        QDateTime wstime = QDateTime::currentDateTime();
        QString teststr = wstime.toString("yyyy-MM-dd hh:mm:ss");
        out << teststr;
        //5.定义size  = buffer.size()
        int size = buffer.size();
        bool newflag = true;

        // 创建共享内存段
        if (!sharedMemory.create(size)) {
            // 从共享内存中读取数据
            if (!sharedMemory.isAttached()) //检测程序当前是否关联共享内存
                sharedMemory.attach();
            QBuffer sbuffer;
            QDataStream in(&sbuffer);
            //读取时间
            QDateTime rstime;
            QString tstr;
            sharedMemory.lock();
            sbuffer.setData((char *)sharedMemory.constData(), sharedMemory.size());
            sbuffer.open(QBuffer::ReadOnly);
            in >> tstr;
            rstime = QDateTime::fromString(tstr, "yyyy-MM-dd hh:mm:ss");
            //sharedMemory.unlock();
            //sharedMemory.detach();
            //no used
//            qint64 temptime = rstime.secsTo(wstime);
            if (!rstime.isValid()) return  0;

            //指为false
            newflag = false;
            //if (sharedMemory.isAttached()) //检测程序当前是否关联共享内存
            //sharedMemory.attach();
            // sharedMemory.lock();
            char *to = (char *)sharedMemory.data();
            const char *from = buffer.data().data();
            memcpy(to, from, qMin(sharedMemory.size(), size));
            sharedMemory.unlock();
            sharedMemory.detach();
            qDebug() << teststr << "   " << tstr << " error " <<  newflag ;

        } else {
            sharedMemory.lock();
            char *to = (char *)sharedMemory.data();
            const char *from = buffer.data().data();
            memcpy(to, from, qMin(sharedMemory.size(), size));
            sharedMemory.unlock();
            qDebug() << teststr;
            qDebug() << "create";
        }
        //save theme
        DApplicationSettings saveTheme;
        CommandLine *cl = CommandLine::instance();
        qDebug() << teststr << "   " << "  " << newflag << "\n";
        if (cl->processOption(wstime, newflag)) {
            return dApp->m_app->exec();
        } else {
            return 0;
        }

    } else {
#ifndef LITE_DIV
        if (!service::isDefaultImageViewer()) {
            qDebug() << "Set defaultImage viewer succeed:" << service::setDefaultImageViewer(true);
        } else {
            qDebug() << "Deepin Image Viewer is defaultImage!";
        }
#endif
        //save theme
        DApplicationSettings saveTheme;
        CommandLine *cl = CommandLine::instance();
        if (cl->processOption()) {
            return dApp->m_app->exec();
        } else {
            return 0;
        }
    }

}
