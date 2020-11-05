/*
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

#ifndef MIMESAPPSMANAGER_H
#define MIMESAPPSMANAGER_H


#include <QObject>
#include <QSet>
#include <QMimeType>
#include <QMap>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QIcon>

class MimesAppsManager: public QObject
{
    Q_OBJECT

public:
    MimesAppsManager(QObject *parent = 0);
    ~MimesAppsManager();

    static QStringList DesktopFiles;
    static QMap<QString, QStringList> MimeApps;
    static QMap<QString, QStringList> DDE_MimeTypes;
    static QString getDefaultAppDesktopFileByMimeType(const QString& mimeType);

    static bool setDefautlAppForTypeByGio(const QString& mimeType,
                                     const QString& targetAppName);
};

#endif // MIMESAPPSMANAGER_H
