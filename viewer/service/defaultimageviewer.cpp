#include "defaultimageviewer.h"
#include "third-party/simpleini/SimpleIni.h"
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

        CSimpleIniA settings;
        settings.SetUnicode(true);
        QString content(utils::base::getFileContent(mimeAppFilePath));
        settings.LoadData(content.toStdString().c_str(), content.length());

        foreach (const QString& mime, supportImageFormat) {
            const char* mime_cstr = mime.toStdString().c_str();

            if (isDefault) {
                settings.SetValue(defaultApplicationsSection, mime_cstr,
                                  appDesktopFile.toStdString().c_str());
                settings.SetValue(addedAssociationsSection, mime_cstr,
                                  appDesktopFile.toStdString().c_str());
            } else {
                const QString appName(settings.GetValue(defaultApplicationsSection,
                                                         mime_cstr));
                if (appName == QString(appDesktopFile)) {
                    settings.Delete(defaultApplicationsSection, mime_cstr);
                }

                const QString appName2(settings.GetValue(addedAssociationsSection, mime_cstr));
                if (appName2 == QString(appDesktopFile)) {
                    settings.Delete(addedAssociationsSection, mime_cstr);
                }
            }
        }
        std::string strData;
        settings.Save(strData);
        return utils::base::writeTextFile(mimeAppFilePath, QString::fromStdString(strData));
    }
}
