// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrinterface.h"
#include <QDBusMetaType>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

OcrInterface::OcrInterface(const QString &serviceName, const QString &ObjectPath,
                           const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(serviceName, ObjectPath, staticInterfaceName(), connection, parent)
{
    qCDebug(logImageViewer) << "OcrInterface instance created";
}

OcrInterface::~OcrInterface()
{
    qCDebug(logImageViewer) << "OcrInterface instance destroyed";
}
