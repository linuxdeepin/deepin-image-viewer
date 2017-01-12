#ifndef VOLUMEMONITOR_H
#define VOLUMEMONITOR_H

#include <QObject>
#include <QSet>
#include <QSocketNotifier>

class VolumeMonitor : public QObject
{
    Q_OBJECT
public:
    static VolumeMonitor *instance();
    bool isRunning();
    ~VolumeMonitor();

signals:
    void deviceAdded(const QString& addDev);
    void deviceRemoved(const QString& removeDe);

public slots:
    bool start();
    bool stop();

private slots:
    void onFileChanged();

private:
    VolumeMonitor(QObject *parent = 0);

private:
    int m_fileKde = -1;
    QSocketNotifier* m_socketNotifier;
    QSet<QString> m_fileContentSet;
    static VolumeMonitor *m_monitor;
};

#endif // VOLUMEMONITOR_H
