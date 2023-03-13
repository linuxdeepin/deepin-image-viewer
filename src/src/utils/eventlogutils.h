// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTLOGUTILS_H
#define EVENTLOGUTILS_H

#include <QJsonObject>
#include <QObject>

#include <string>

class Eventlogutils : public QObject
{
    Q_OBJECT

public:
    enum EventTID {
        OpenTime = 1000000000,
        CloseTime = 1000000001,
        StartUp = 1000000003,
        Quit = 1000000004,
    };

    static Eventlogutils *GetInstance();
    void writeLogs(const QJsonObject &data);
    void sendLogs(const QJsonObject &data);
    bool sendLogsEnabled() const;

    Q_SLOT void forwardLogData(const QJsonObject &data);

private:
    static Eventlogutils *m_pInstance;
    Eventlogutils();
    ~Eventlogutils() override = default;

    bool (*initFunc)(const std::string &packagename, bool enable_sig) = nullptr;
    void (*writeEventLogFunc)(const std::string &eventdata) = nullptr;
    void (*sendEventLogFunc)(const std::string &eventdata) = nullptr;

    Q_DISABLE_COPY(Eventlogutils)
};

#endif  // EVENTLOGUTILS_H
