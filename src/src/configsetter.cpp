    // SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsetter.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <DLog>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

const QString CONFIG_PATH = QDir::homePath() + "/.config/deepin/deepin-image-viewer/config.conf";

LibConfigSetter::LibConfigSetter(QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "LibConfigSetter constructor called.";
    m_settings = new QSettings(CONFIG_PATH, QSettings::IniFormat, this);
    qCDebug(logImageViewer) << "QSettings initialized with path: " << CONFIG_PATH;
}

LibConfigSetter::~LibConfigSetter() {
    qCDebug(logImageViewer) << "LibConfigSetter destructor called.";
}

LibConfigSetter *LibConfigSetter::m_setter = nullptr;
LibConfigSetter *LibConfigSetter::instance()
{
    qCDebug(logImageViewer) << "LibConfigSetter::instance() called.";
    if (!m_setter) {
        qCDebug(logImageViewer) << "Creating new LibConfigSetter instance.";
        m_setter = new LibConfigSetter();
    }

    return m_setter;
}

void LibConfigSetter::setValue(const QString &group, const QString &key, const QVariant &value)
{
    qCDebug(logImageViewer) << "LibConfigSetter::setValue() called for group: " << group << ", key: " << key << ", value: " << value;
    m_settings->beginGroup(group);
    qCDebug(logImageViewer) << "Settings beginGroup: " << group;
    m_settings->setValue(key, value);
    qCDebug(logImageViewer) << "Settings setValue: " << key << " = " << value;
    m_settings->endGroup();
    qCDebug(logImageViewer) << "Settings endGroup: " << group;

    emit valueChanged(group, key, value);
    qCDebug(logImageViewer) << "Emitted valueChanged signal.";
}

QVariant LibConfigSetter::value(const QString &group, const QString &key, const QVariant &defaultValue)
{
    qCDebug(logImageViewer) << "LibConfigSetter::value() called for group: " << group << ", key: " << key << ", defaultValue: " << defaultValue;
    QMutexLocker locker(&m_mutex);
    qCDebug(logImageViewer) << "Mutex locked.";

    QVariant value;
    m_settings->beginGroup(group);
    qCDebug(logImageViewer) << "Settings beginGroup: " << group;
    value = m_settings->value(key, defaultValue);
    qCDebug(logImageViewer) << "Settings value retrieved: " << key << " = " << value;
    m_settings->endGroup();
    qCDebug(logImageViewer) << "Settings endGroup: " << group;

    qCDebug(logImageViewer) << "Mutex unlocked, returning value: " << value;
    return value;
}
