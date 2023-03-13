// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventlogutils.h"

#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>
#include <QDebug>

Eventlogutils *Eventlogutils::m_pInstance = nullptr;
Eventlogutils *Eventlogutils::GetInstance()
{
    if (m_pInstance == nullptr) {
        m_pInstance  = new Eventlogutils();
    }
    return m_pInstance;
}

void Eventlogutils::writeLogs(const QJsonObject &data)
{
    if (!writeEventLogFunc)
        return;

    writeEventLogFunc(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}

void Eventlogutils::sendLogs(const QJsonObject &data)
{
    if (!sendEventLogFunc)
        return;

    sendEventLogFunc(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}

/**
   @return 返回当前 sendLogs() 接口是否可用，不同 EventLog 版本支持的接口不同，
        旧版本仅提供 writeLogs()
 */
bool Eventlogutils::sendLogsEnabled() const
{
    return  nullptr != sendEventLogFunc;
}

/**
   @brief 转发外部发送的埋点数据 `data`，此接口仅用于1.2及之后版本的数据埋点
 */
void Eventlogutils::forwardLogData(const QJsonObject &data)
{
    qInfo() << qPrintable("EventLog:") << data;
    sendLogs(data);
}

Eventlogutils::Eventlogutils()
{
    QLibrary library("libdeepin-event-log.so");
    initFunc = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLogFunc = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));
    sendEventLogFunc = reinterpret_cast<void (*)(const std::string &)>(library.resolve("SendEventLog"));

    if (!initFunc) {
        qWarning() << qPrintable("Not resolve Eventlog api Initialize()");
        return;
    }

    if (!writeEventLogFunc) {
        qWarning() << qPrintable("Not resolve Eventlog api WriteEventLog()");
    }
    if (!sendEventLogFunc) {
        qWarning() << qPrintable("Not resolve Eventlog api SendEventLog()");
    }

    initFunc("deepin-image-viewer", true);
}
