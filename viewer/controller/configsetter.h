#ifndef CONFIGSETTER_H
#define CONFIGSETTER_H

#include <QObject>
#include <QSettings>

class ConfigSetter : public QObject
{
    Q_OBJECT
public:
    static ConfigSetter *instance();
    void setValue(const QString &group, const QString &key,
                  const QVariant &value);
    QVariant value(const QString &group, const QString &key,
                   const QVariant &defaultValue = QVariant());

private:
    explicit ConfigSetter(QObject *parent = 0);

private:
    static ConfigSetter *m_setter;
    QSettings *m_settings;
};

#endif // CONFIGSETTER_H
