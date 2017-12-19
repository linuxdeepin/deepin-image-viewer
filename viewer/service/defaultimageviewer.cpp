/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#include "defaultimageviewer.h"
#include "utils/baseutils.h"
#include <QStandardPaths>
#include <QFile>
#include <QSettings>
#include <QDebug>

namespace service {
    //Config file used to setup default applications for current desktop.
    //That is $HOME/.config/mimeapps.list
    const char mimeAppFileName[] = "mimeapps.list";

    //Section names in mimeapps.list
    const char defaultApplicationsSection[] = "Default Applications";
    const char addedAssociationsSection[] = "Added Associations";
    const QString appDesktopFile = "deepin-image-viewer.desktop";
    const QStringList supportImageFormat = {
        "image/jpeg",
    };

    QString getMimeAppPath() {
        QString configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).at(0);
        QString mimeAppPath = QString("%1/%2").arg(configPath).arg(QString(mimeAppFileName));
        return mimeAppPath;
    }

    bool isDefaultImageViewer() {
        const QString mimeAppFilePath(getMimeAppPath());

        if (!QFile::exists(mimeAppFilePath)) {
            return false;
        }

        bool state = true;
        QSettings settings(mimeAppFilePath, QSettings::IniFormat);

        settings.beginGroup(defaultApplicationsSection);
        foreach (const QString& mime, supportImageFormat) {
            QString appName = settings.value(mime).toString();
            if (appName != appDesktopFile) {
                state = false;
                break;
            }
        }
        settings.endGroup();
        return state;
    }

    bool setDefaultImageViewer(bool isDefault) {
        QString mimeAppFilePath(getMimeAppPath());
        if (!isDefault &&!QFile::exists(mimeAppFilePath)) {
            return false;
        }

        QSettings settings(mimeAppFilePath, QSettings::IniFormat);

        foreach (const QString& mime, supportImageFormat) {
            if (isDefault) {
                settings.beginGroup(defaultApplicationsSection);
                settings.setValue(mime, appDesktopFile);
                settings.endGroup();
                settings.sync();
                settings.beginGroup(addedAssociationsSection);
                settings.setValue(mime, appDesktopFile);
                settings.endGroup();
                settings.sync();
            } else {
                settings.beginGroup(defaultApplicationsSection);
                const QString appName = settings.value(mime).toString();

                if (appName == QString(appDesktopFile)) {
                    settings.beginGroup(defaultApplicationsSection);
                    settings.remove(mime);
                    settings.endGroup();
                }

                settings.beginGroup(addedAssociationsSection);
                const QString appName2 = settings.value(mime).toString();
                if (appName2 == QString(appDesktopFile)) {
                    settings.remove(mime);
                }
            }
        }

        return true;
    }
}
