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

#include "mimesappsmanager.h"

#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QDirIterator>
#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

#undef signals
extern "C" {
    #include <gio/gio.h>
    #include <gio/gdesktopappinfo.h>
    #include <gio/gappinfo.h>
}
#define signals public

QStringList MimesAppsManager::DesktopFiles = {};
QMap<QString, QStringList> MimesAppsManager::MimeApps = {};
QMap<QString, QStringList> MimesAppsManager::DDE_MimeTypes = {};

MimesAppsManager::MimesAppsManager(QObject *parent): QObject(parent)
{
}

MimesAppsManager::~MimesAppsManager()
{
}

QString MimesAppsManager::getDefaultAppDesktopFileByMimeType(const QString &mimeType)
{
    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    if(!defaultApp)
        return "";

    const char* desktop_id = g_app_info_get_id(defaultApp);
    GDesktopAppInfo* desktopAppInfo = g_desktop_app_info_new(desktop_id);
    if(!desktopAppInfo)
        return "";
    QString desktopFile = g_desktop_app_info_get_filename(desktopAppInfo);

    g_object_unref(defaultApp);
    g_object_unref(desktopAppInfo);

    return desktopFile;
}

bool MimesAppsManager::setDefautlAppForTypeByGio(const QString &mimeType, const QString &appPath)
{
    GAppInfo* app = NULL;
    GList* apps = NULL;
    apps = g_app_info_get_all();

    GList* iterator = apps;

    while (iterator) {
        const char* desktop_id = g_app_info_get_id((GAppInfo*)iterator->data);
        GDesktopAppInfo* desktopAppInfo = g_desktop_app_info_new(desktop_id);

        if (desktopAppInfo) {
            if (appPath == g_desktop_app_info_get_filename(desktopAppInfo)) {
                app = (GAppInfo*)iterator->data;
                g_object_unref(desktopAppInfo);
                break;
            }

            g_object_unref(desktopAppInfo);
        } else if (appPath.endsWith("/" + QString::fromLocal8Bit(desktop_id))) {
            app = (GAppInfo*)iterator->data;
            break;
        }

        iterator = iterator->next;
    }

    g_list_free(apps);

    if (!app) {
        qWarning() << "no app found name as:" << appPath;
        return false;
    }

    GError* error = NULL;
    g_app_info_set_as_default_for_type(app,
                                       mimeType.toLocal8Bit().constData(),
                                       &error);
    if (error) {
        qDebug () << "fail to set default app for type:" << error->message;
        g_free(error);
        return false;
    }

    return true;
}



