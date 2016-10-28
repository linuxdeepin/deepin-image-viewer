#ifndef VOLUMEMONITOR_H
#define VOLUMEMONITOR_H

#include <QObject>
#include <QSet>
#include <QSocketNotifier>

class VolumeMonitor : public QObject
{
    Q_OBJECT
public:
    VolumeMonitor(QObject *parent = 0);
    ~VolumeMonitor();

signals:
    void deviceAdded(const QString& addDev);
    void deviceRemoved(const QString& removeDe);

public slots:
    bool start();
    bool stop();
    bool isRunning();
    void onFileChanged();
private:
    int m_fileKde = -1;
    QSocketNotifier* m_socketNotifier;
    QSet<QString> m_fileContentSet;
};

#endif // VOLUMEMONITOR_H
