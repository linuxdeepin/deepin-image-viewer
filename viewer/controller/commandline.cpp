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
#include "commandline.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "controller/divdbuscontroller.h"
#include "controller/configsetter.h"
#include "service/deepinimageviewerdbus.h"
#include "frame/mainwindow.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"

#include <QCommandLineOption>
#include <QDBusConnection>
#include <QDesktopWidget>
#include <QDebug>
#include <QFileInfo>

using namespace Dtk::Widget;

namespace {

const QString DBUS_PATH = "/com/deepin/ImageViewer";
const QString DBUS_NAME = "com.deepin.ImageViewer";
const QString THEME_GROUP = "APP";
const QString THEME_TEXT = "AppTheme";
}

struct CMOption {
    QString shortOption;
    QString longOption;
    QString description;
    QString valueName;
};

static CMOption options[] = {
    {"o", "open", "Open the specified <image-file>.", "image-file"},
    {"a", "album", "Enter the album <album-name>.", "album-name"},
//    {"s", "search", "Go to search view and search image by <word>.", "word"},
//    {"e", "edit", "Go to edit view and begin editing <image-file>.", "image-file"},
    {"w", "wallpaper", "Set <image-file> as wallpaper.", "image-file"},
    {"new-window", "new-window", "Display a window.", ""},
    {"", "", "", ""}
};

CommandLine *CommandLine::m_commandLine = nullptr;
CommandLine *CommandLine::instance()
{
    if (! m_commandLine) {
        m_commandLine = new CommandLine();
    }

    return m_commandLine;
}

CommandLine::CommandLine()
{
    m_cmdParser.addHelpOption();

    for (const CMOption *i = options; ! i->shortOption.isEmpty(); ++i) {
        addOption(i);
    }
}

CommandLine::~CommandLine()
{

}
QString CommandLine::createOpenImageInfo(QString path, QStringList pathlist, QDateTime stime)
{
    QJsonObject json;

    QString str = stime.toString("yyyy-MM-ddThh:mm:ss");
    json.insert("OpenTime", str);
    json.insert("ImagePath", path);
    QJsonArray jsonarry;
    for (int i = 0; i < pathlist.count(); i++) {
        jsonarry.append(pathlist.at(i));
    }
    json.insert("ImagePathList", jsonarry);
    // 构建 JSON 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    return strJson;
}

void CommandLine::paraOpenImageInfo(QString source, QString &path, QStringList &pathlist)
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(source.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return;
    }
    QJsonObject rootObj = jsonDoc.object();

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("ImagePath")) {
        path = rootObj.value("ImagePath").toString();
    }
    if (rootObj.contains("ImagePathList")) {
        QJsonArray subArray = rootObj.value("ImagePathList").toArray();
        for (int i = 0; i < subArray.size(); i++) {
            QString subObj = subArray.at(i).toString();
            pathlist.append(subObj);
        }
    }
}

void CommandLine::addOption(const CMOption *option)
{
    QStringList ol;
    ol << option->shortOption;
    ol << option->longOption;
    QCommandLineOption cm(ol, option->description, option->valueName);

    m_cmdParser.addOption(cm);
}

/*!
 * \brief CommandLine::showHelp
 * QCommandLineParser::showHelp(int exitCode = 0) Will displays the help
 * information, and exits application automatically. However,
 * DApplication::loadDXcbPlugin() need exit by calling quick_exit(a.exec()).
 * So we should show the help message only by calling this function.
 */
void CommandLine::showHelp()
{
    fputs(qPrintable(m_cmdParser.helpText()), stdout);
}

void CommandLine::viewImage(const QString path, const QStringList paths)
{
    if (!QFileInfo(path).exists()) {
        dApp->m_timer=1000;
    }
    MainWindow *w = new MainWindow(false);
    w->setWindowRadius(18);
    w->setBorderWidth(0);
    w->show();

    // Load image after all UI elements has been init
    // BottomToolbar pos not correct on init
    emit dApp->signalM->hideBottomToolbar(true);
    emit dApp->signalM->enableMainMenu(false);

//    QTimer::singleShot(300, this, [ = ] {
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("com.deepin.ImageViewer");
    dbus.registerObject("/com/deepin/ImageViewer", w);
    SignalManager::ViewInfo info;
    info.album = "";
#ifndef LITE_DIV
    info.inDatabase = false;
#endif
    info.lastPanel = nullptr;
    info.path = path;
    info.paths = paths;

    emit dApp->signalM->viewImage(info);
//    });
}

bool CommandLine::processOption()
{
    if (! m_cmdParser.parse(dApp->arguments())) {
        showHelp();
        return false;
    }

    QString defaulttheme = dApp->setter->value(THEME_GROUP,
                                               THEME_TEXT).toString();

    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
    } else {
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
    }

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
    [](DGuiApplicationHelper::ColorType type) {
        Q_UNUSED(type);

    });

    QStringList names = m_cmdParser.optionNames();
    QStringList pas = m_cmdParser.positionalArguments();
    qDebug() << "names pas" << names << pas;
#ifndef LITE_DIV
    DeepinImageViewerDBus *dvd = new DeepinImageViewerDBus(dApp->signalM);
    if (names.isEmpty() && pas.isEmpty()) {
        if (QDBusConnection::sessionBus().registerService(DBUS_NAME) &&
                QDBusConnection::sessionBus().registerObject(DBUS_PATH,
                                                             dvd, QDBusConnection::ExportScriptableSlots)) {
            MainWindow *w = new MainWindow(true);
            w->show();
            emit dApp->signalM->backToMainPanel();

            return true;
        } else {
            DIVDBusController().activeWindow();
            //delay 1 second to exit process
            QTimer::singleShot(1000, [ = ] {
                dApp->quit();
            });

            qDebug() << "Deepin Image Viewer is running...";
            return false;
        }
    } else {
        DIVDBusController *dc = new DIVDBusController(dApp->signalM);
        Q_UNUSED(dc)
#endif

        using namespace utils::image;
        QString name;
        QString value;
        QStringList values;
        if (! names.isEmpty()) {
            name = names.first();
            value = m_cmdParser.value(name);
            values = m_cmdParser.values(name);
        }
        if (values.isEmpty() && ! pas.isEmpty()) {
            QString path = pas.first();
            qDebug() << "path=" << path;

            bool isFile = false;
            if (QFileInfo(path).isDir()) {
                isFile = true;
                qDebug() << "Yes";
            }

            if (isFile) {
                qDebug() << "isFile";

                name = "new-window";

                QDir dir(path);
                if (!dir.exists()) {
                    qDebug() << "!dir.exists()";
                    return false;
                }

                dir.setFilter(QDir::Files | QDir::NoSymLinks);
                QFileInfoList list = dir.entryInfoList();

                int file_count = list.count();
                if (file_count <= 0) {
                    qDebug() << "file_count <=0";
                    return false;
                }

                QStringList string_list;
                for (int i = 0; i < list.count(); i++) {
                    QFileInfo file_info = list.at(i);
                    QString absolute_file_path = file_info.absoluteFilePath();
                    if (QFileInfo(absolute_file_path).exists() /*&& imageSupportRead(absolute_file_path)*/) {
                        string_list << absolute_file_path;
                    }
                }
                value = string_list.first();
                values = string_list;
                qDebug() << "isFile"
                         << "name" << name
                         << "value" << value
                         << "values" << values;
            } else {
                name = "o"; // Default operation is open image file
                value = pas.first();

                if (QUrl(value).isLocalFile()) {
                    value =  QUrl(value).toLocalFile();
                }
                values = pas;
            }
        }
        bool support = imageSupportRead(value);

        if (name == "o" || name == "open") {
            if (values.length() > 1) {
                QStringList aps;
                for (QString path : values) {
                    if (QUrl(value).isLocalFile())
                        path =  QUrl(value).toLocalFile();
                    const QString ap = QFileInfo(path).absoluteFilePath();
                    if (QFileInfo(path).exists() && imageSupportRead(ap)) {
                        aps << ap;
                    }
                }
                if (! aps.isEmpty()) {
                    viewImage(aps.first(), aps);
                    return true;
                } else {
                    return false;
                }
            } else if (support) {
                viewImage(QFileInfo(value).absoluteFilePath(), QStringList());
                return true;
            } else {
                return false;
            }
        }
#ifndef LITE_DIV
        else if (name == "a" || name == "album") {
            dc->enterAlbum(value);
        } else if (name == "s" || name == "search") {
            dc->searchImage(value);
        } else if ((name == "e" || name == "edit") && support) {
            dc->editImage(QFileInfo(value).absoluteFilePath());
        }
#endif
        else if ((name == "w" || name == "wallpaper") && support) {
            qDebug() << "Set " << value << " as wallpaper.";
            dApp->wpSetter->setWallpaper(QFileInfo(value).absoluteFilePath());
        } else if (name == "new-window") {
            qDebug() << "new-window" << value << "file";
            viewImage(value, values);
            return true;
        } else if (name.isEmpty()) {
            viewImage("", {});
            return true;
        } else {
            showHelp();
        }

        return false;
#ifndef LITE_DIV
    }
#endif
}

bool CommandLine::processOption(QDateTime time, bool newflag)
{
    if (! m_cmdParser.parse(dApp->arguments())) {
        showHelp();
        return false;
    }

    QString defaulttheme = dApp->setter->value(THEME_GROUP,
                                               THEME_TEXT).toString();

    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
    } else {
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
    }

//    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
//    [](DGuiApplicationHelper::ColorType type) {
//        Q_UNUSED(type);
////        if(DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType() ){
////            dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
////        } else {
////            dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
////        }
//    });


    QStringList names = m_cmdParser.optionNames();
    QStringList pas = m_cmdParser.positionalArguments();
    qDebug() << "names pas" << names << pas;
#ifndef LITE_DIV
    DeepinImageViewerDBus *dvd = new DeepinImageViewerDBus(dApp->signalM);
    if (names.isEmpty() && pas.isEmpty()) {
        if (QDBusConnection::sessionBus().registerService(DBUS_NAME) &&
                QDBusConnection::sessionBus().registerObject(DBUS_PATH,
                                                             dvd, QDBusConnection::ExportScriptableSlots)) {
            MainWindow *w = new MainWindow(true);
            w->show();
            emit dApp->signalM->backToMainPanel();

            return true;
        } else {
            DIVDBusController().activeWindow();
            //delay 1 second to exit process
            QTimer::singleShot(1000, [ = ] {
                dApp->quit();
            });

            qDebug() << "Deepin Image Viewer is running...";
            return false;
        }
    } else {
        DIVDBusController *dc = new DIVDBusController(dApp->signalM);
        Q_UNUSED(dc)
    }
#endif
        using namespace utils::image;
        QString name;
        QString value;
        QStringList values;
        if (! names.isEmpty()) {
            name = names.first();
            value = m_cmdParser.value(name);
            values = m_cmdParser.values(name);
        }

        if (values.isEmpty() && ! pas.isEmpty()) {
            QString path = pas.first();
            qDebug() << "path=" << path;

            bool isFile = false;
            if (QFileInfo(path).isDir()) {
                isFile = true;
                qDebug() << "Yes";
            }

            if (isFile) {
                qDebug() << "isFile";

                name = "new-window";

                QDir dir(path);
                if (!dir.exists()) {
                    qDebug() << "!dir.exists()";
                    return false;
                }

                dir.setFilter(QDir::Files | QDir::NoSymLinks);
                QFileInfoList list = dir.entryInfoList();

                int file_count = list.count();
                if (file_count <= 0) {
                    qDebug() << "file_count <=0";
                    return false;
                }

                QStringList string_list;
                for (int i = 0; i < list.count(); i++) {
                    QFileInfo file_info = list.at(i);
                    QString absolute_file_path = file_info.absoluteFilePath();
                    if (QFileInfo(absolute_file_path).exists() && imageSupportRead(absolute_file_path)) {
                        string_list << absolute_file_path;
                    }
                }
                value = string_list.first();
                values = string_list;
                qDebug() << "isFile"
                         << "name" << name
                         << "value" << value
                         << "values" << values;
            } else {
                name = "o"; // Default operation is open image file
                value = pas.first();

                if (QUrl(value).isLocalFile()) {
                    value =  QUrl(value).toLocalFile();
                }
                values = pas;
            }
        }

        bool support = suffixisImage(value);

        if (newflag) {
            if (name == "o" || name == "open") {
                if (values.length() > 1) {
                    QStringList aps;
                    for (QString path : values) {
                        if (QUrl(value).isLocalFile())
                            path =  QUrl(value).toLocalFile();
                        const QString ap = QFileInfo(path).absoluteFilePath();
                        if (QFileInfo(path).exists() && suffixisImage(ap)) {
                            aps << ap;
                        }
                    }
                    if (! aps.isEmpty()) {
                        viewImage(aps.first(), aps);
                        return true;
                    } else {
                        return false;
                    }
                } else if (support) {
                    viewImage(QFileInfo(value).absoluteFilePath(), QStringList());
                    return true;
                } else {
                    return false;
                }
            }
#ifndef LITE_DIV
            else if (name == "a" || name == "album") {
                dc->enterAlbum(value);
            } else if (name == "s" || name == "search") {
                dc->searchImage(value);
            } else if ((name == "e" || name == "edit") && support) {
                dc->editImage(QFileInfo(value).absoluteFilePath());
            }
#endif
            else if ((name == "w" || name == "wallpaper") && support) {
                qDebug() << "Set " << value << " as wallpaper.";
                dApp->wpSetter->setWallpaper(QFileInfo(value).absoluteFilePath());
            } else if (name == "new-window") {
                qDebug() << "new-window" << value << "file";
                viewImage(value, values);
                return true;
            } else if (name.isEmpty()) {
                viewImage("", {});
                return true;
            } else {
                showHelp();
            }
            return false;
        } else {
            // show deepin-music
            QDBusInterface iface("com.deepin.ImageViewer",
                                 "/com/deepin/ImageViewer",
                                 "com.deepin.ImageViewer",
                                 QDBusConnection::sessionBus());
            if (iface.isValid()) {
                if (name == "o" || name == "open") {
                    if (values.length() > 1) {
                        QStringList aps;
                        for (QString path : values) {
                            if (QUrl(value).isLocalFile())
                                path =  QUrl(value).toLocalFile();
                            const QString ap = QFileInfo(path).absoluteFilePath();
                            if (QFileInfo(path).exists() && imageSupportRead(ap)) {
                                aps << ap;
                            }
                        }
                        if (! aps.isEmpty()) {
                            iface.asyncCall("OpenImage", createOpenImageInfo(aps.first(), aps, time));
                            iface.asyncCall("OpenImage",  QFileInfo(value).absoluteFilePath());
                            //viewImage(aps.first(), aps);
                        }
                    } else if (support) {
                        iface.asyncCall("OpenImage", createOpenImageInfo(QFileInfo(value).absoluteFilePath(), QStringList(), time));
                        //viewImage(QFileInfo(value).absoluteFilePath(), QStringList());
                    }
                } else if ((name == "w" || name == "wallpaper") && support) {
                    qDebug() << "Set " << value << " as wallpaper.";
                    dApp->wpSetter->setWallpaper(QFileInfo(value).absoluteFilePath());
                }


            }

            return false;
        }

    }

