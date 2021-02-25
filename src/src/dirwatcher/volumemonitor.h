///*
// * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program.  If not, see <http://www.gnu.org/licenses/>.
// */
//#ifndef VOLUMEMONITOR_H
//#define VOLUMEMONITOR_H

//#include <QObject>
//#include <QSet>
//#include <QSocketNotifier>

//class VolumeMonitor : public QObject
//{
//    Q_OBJECT
//public:
//    static VolumeMonitor *instance();
//    bool isRunning();
//    ~VolumeMonitor();

//    QString getMountPoint(const QString &record);
//signals:
//    void deviceAdded(const QString& addDev);
//    void deviceRemoved(const QString& removeDe);

//public slots:
//    bool start();
//    bool stop();

//private slots:
//    void onFileChanged();

//private:
//    explicit VolumeMonitor(QObject *parent = nullptr);

//private:
//    int m_fileKde = -1;
//    QSocketNotifier* m_socketNotifier;
//    QSet<QString> m_fileContentSet;
//    static VolumeMonitor *m_monitor;
//};

//#endif // VOLUMEMONITOR_H
