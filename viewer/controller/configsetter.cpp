#include "configsetter.h"
#include <QDebug>

ConfigSetter::ConfigSetter(QObject *parent) : QObject(parent)
{
    m_settings = new  QSettings("Deepin","DeepinImageViewer", this);
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

void ConfigSetter::setValue(const QString &group, const QString &key,
                            const QVariant &value)
{
    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();

    emit valueChanged(group, key, value);
}

QVariant ConfigSetter::value(const QString &group, const QString &key,
                             const QVariant &defaultValue)
{
    QVariant value;
    m_settings->beginGroup(group);
    value = m_settings->value(key, defaultValue);
    m_settings->endGroup();

    return value;
}

QStringList ConfigSetter::keys(const QString &group)
{
    QStringList v;
    m_settings->beginGroup(group);
    v = m_settings->childKeys();
    m_settings->endGroup();

    return v;
}
