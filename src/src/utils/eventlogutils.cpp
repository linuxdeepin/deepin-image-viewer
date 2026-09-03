// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventlogutils.h"
#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>
#include <DLog>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

Eventlogutils *Eventlogutils::m_pInstance = nullptr;
Eventlogutils *Eventlogutils::GetInstance()
{
    qCDebug(logImageViewer) << "Getting Eventlogutils instance.";
    if (m_pInstance == nullptr) {
        m_pInstance  = new Eventlogutils();
        qCDebug(logImageViewer) << "Created new Eventlogutils instance.";
    }
    return m_pInstance;
}

void Eventlogutils::writeLogs(QJsonObject &data)
{
    qCDebug(logImageViewer) << "Attempting to write event logs.";
    if (!writeEventLogFunc) {
        qCWarning(logImageViewer) << "writeEventLogFunc is not initialized. Cannot write logs.";
        return;
    }

    writeEventLogFunc(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
    qCDebug(logImageViewer) << "Event logs written successfully.";
}

Eventlogutils::Eventlogutils()
{
    qCDebug(logImageViewer) << "Initializing Eventlogutils.";
    QLibrary library("libdeepin-event-log.so");
    initFunc = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    if (!initFunc) {
        qCWarning(logImageViewer) << "Failed to resolve Initialize function from libdeepin-event-log.so.";
        return;
    }

    // Initialize 解析成功后再解析写入函数，保证 writeEventLogFunc 就绪时事件库必然可用
    writeEventLogFunc = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));
    if (!writeEventLogFunc) {
        qCWarning(logImageViewer) << "Failed to resolve WriteEventLog function from libdeepin-event-log.so.";
        return;
    }

    if (!initFunc("deepin-image-viewer", true)) {
        qCWarning(logImageViewer) << "Failed to initialize libdeepin-event-log.so, event logging disabled.";
        writeEventLogFunc = nullptr;
        return;
    }
    qCDebug(logImageViewer) << "Event logging initialized for deepin-image-viewer.";
}
