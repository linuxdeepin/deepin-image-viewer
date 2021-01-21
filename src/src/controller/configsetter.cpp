/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#include "configsetter.h"
#include <QDebug>

#include <QDir>
#include <QFileInfo>
#include <QProcess>

const QString CONFIG_PATH =   QDir::homePath() +
                              "/.config/deepin/deepin-image-viewer/config.conf";
const QString DB_PATH = QDir::homePath() +
                        "/.local/share/deepin/deepin-image-viewer/deepinimageviewer.db";

ConfigSetter::ConfigSetter(QObject *parent) : QObject(parent)
{
    if (!QFileInfo(CONFIG_PATH).exists())
        QProcess::startDetached(QString("rm %1").arg(DB_PATH));

    m_settings = new QSettings(CONFIG_PATH, QSettings::IniFormat, this);
    qDebug() << "Setting file:" << m_settings->fileName();
}

ConfigSetter *ConfigSetter::m_setter = nullptr;
ConfigSetter *ConfigSetter::instance()
{
    if (! m_setter) {
        m_setter = new ConfigSetter();
    }

    return m_setter;
}

void ConfigSetter::setValue(const QString& group, const QString& key,
                            const QVariant& value)
{
//    QMutexLocker locker(&m_mutex);

    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();

    emit valueChanged(group, key, value);
}

QVariant ConfigSetter::value(const QString& group, const QString& key,
                             const QVariant& defaultValue)
{
    QMutexLocker locker(&m_mutex);

    QVariant value;
    m_settings->beginGroup(group);
    value = m_settings->value(key, defaultValue);
    m_settings->endGroup();

    return value;
}

//QStringList ConfigSetter::keys(const QString group)
//{
//    QStringList v;
//    m_settings->beginGroup(group);
//    v = m_settings->childKeys();
//    m_settings->endGroup();

//    return v;
//}
