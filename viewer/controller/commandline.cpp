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

#include "dthememanager.h"

#include <QCommandLineOption>
#include <QDBusConnection>
#include <QDesktopWidget>
#include <QDebug>
#include <QFileInfo>

using namespace Dtk::Widget;

namespace {

const QString DBUS_PATH = "/com/deepin/DeepinImageViewer";
const QString DBUS_NAME = "com.deepin.DeepinImageViewer";
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
//    m_cmdParser.addVersionOption();
//    m_cmdParser.addPositionalArgument("value", QCoreApplication::translate(
//        "main", "Value that use for options."), "[value]");

    for (const CMOption* i = options; ! i->shortOption.isEmpty(); ++i) {
        addOption(i);
    }
}

CommandLine::~CommandLine() {

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

void CommandLine::viewImage(const QString &path, const QStringList &paths)
{
    MainWindow *w = new MainWindow(false);
    w->show();
    // Load image after all UI elements has been init
    // BottomToolbar pos not correct on init
    emit dApp->signalM->hideBottomToolbar(true);
    emit dApp->signalM->enableMainMenu(false);

    TIMER_SINGLESHOT(50, {
    SignalManager::ViewInfo vinfo;
    vinfo.album = "";
    vinfo.inDatabase = false;
    vinfo.lastPanel = nullptr;
    vinfo.path = path;
    vinfo.paths = paths;
    emit dApp->signalM->viewImage(vinfo);
                     }, path, paths)
}

bool CommandLine::processOption()
{
    if (! m_cmdParser.parse(dApp->arguments())) {
        showHelp();
        return false;
    }

    QString defaulttheme = dApp->setter->value(THEME_GROUP,
                                                   THEME_TEXT).toString();
    if (defaulttheme.isEmpty()||defaulttheme == "Light") {
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
        Dtk::Widget::DThemeManager::instance()->setTheme("light");
    } else {
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
        Dtk::Widget::DThemeManager::instance()->setTheme("dark");
    }

    QStringList names = m_cmdParser.optionNames();
    QStringList pas = m_cmdParser.positionalArguments();
    if (names.isEmpty() && pas.isEmpty()) {
        if (QDBusConnection::sessionBus().registerService(DBUS_NAME) &&
                QDBusConnection::sessionBus().registerObject(DBUS_PATH, dApp->signalM)) {
            MainWindow *w = new MainWindow(true);
            w->show();
            emit dApp->signalM->backToMainPanel();

            return true;
        }
        else {
            DIVDBusController().activeWindow();
            //delay 1 second to exit process
            QTimer::singleShot(1000, [=]{
                dApp->quit();
            });

            qDebug() << "Deepin Image Viewer is running...";
            return false;
        }
    }
    else {
        using namespace utils::image;
        DIVDBusController *dc = new DIVDBusController(dApp->signalM);
        Q_UNUSED(dc)

        QString name;
        QString value;
        QStringList values;
        if (! names.isEmpty()) {
            name = names.first();
            value = m_cmdParser.value(name);
            values = m_cmdParser.values(name);
        }
        else if (! pas.isEmpty()){
            name = "o"; // Default operation is open image file
            value = pas.first();

            if (QUrl(value).isLocalFile()) {
                value =  QUrl(value).toLocalFile();
            }
            values = pas;
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
                }
                else {
                    return false;
                }
            }
            else if (QFileInfo(value).exists() &&
                     imageSupportRead(QFileInfo(value).absoluteFilePath())) {
                viewImage(QFileInfo(value).absoluteFilePath(), QStringList());
                return true;
            }
            else {
                return false;
            }
        }
        else if (name == "a" || name == "album") {
            dc->enterAlbum(value);
        }
        else if (name == "s" || name == "search") {
            dc->searchImage(value);
        }
        else if ((name == "e" || name == "edit") && support) {
            dc->editImage(QFileInfo(value).absoluteFilePath());
        }
        else if ((name == "w" || name == "wallpaper") && support) {
            qDebug() << "Set " << value << " as wallpaper.";
            dApp->wpSetter->setWallpaper(QFileInfo(value).absoluteFilePath());
        }
        else {
            showHelp();
        }

        return false;
    }
}
