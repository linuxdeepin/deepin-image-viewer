#include "commandline.h"
#include "frame/mainwindow.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "controller/divdbuscontroller.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QDBusConnection>
#include <QDebug>

namespace {

const QString DBUS_PATH = "/com/deepin/deepinimageviewer";
const QString DBUS_NAME = "com.deepin.deepinimageviewer";

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
    {"s", "search", "Go to search view and search image by <word>.", "word"},
    {"e", "edit", "Go to edit view and begin editing <image-file>.", "image-file"},
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
    m_cmdParser.setApplicationDescription("Deepin Image Viewer");
    m_cmdParser.addHelpOption();
    m_cmdParser.addVersionOption();

    for (const CMOption* i = options; ! i->shortOption.isEmpty(); ++i) {
        addOption(i);
    }

    m_cmdParser.process(*qApp);

    int oc = m_cmdParser.optionNames().length();
    if (oc > 1) {
        m_cmdParser.showHelp();
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

bool CommandLine::processOption()
{
    SignalManager *sm = SignalManager::instance();
    DeepinImageViewerDBus *dd = new DeepinImageViewerDBus(sm);
    Q_UNUSED(dd);

    QStringList names = m_cmdParser.optionNames();
    if (names.isEmpty()) {
        if (QDBusConnection::sessionBus().registerService(DBUS_NAME) &&
                QDBusConnection::sessionBus().registerObject(DBUS_PATH, sm)) {
            MainWindow *w = new MainWindow;
            w->show();
            emit sm->backToMainWindow();

            return true;
        }
        else {
            qDebug() << "Deepin Image Viewer is running...";
            return false;
        }
    }
    else {
        DIVDBusController *dc = new DIVDBusController(sm);

        QString name = names.first();
        QString value = m_cmdParser.value(names.first());

        if (name == "o" || name == "open") {
            qDebug() << "Open image file: " << value;
            MainWindow *w = new MainWindow;
            w->show();
            emit sm->viewImage(value, "", true);
            return true;
        }
        else if (name == "a" || name == "album") {
            dc->enterAlbum(value);
        }
        else if (name == "s" || name == "search") {
            dc->searchImage(value);
        }
        else if (name == "e" || name == "edit") {
            dc->editImage(value);
        }
        else if (name == "w" || name == "wallpaper") {
            qDebug() << "Set " << value << " as wallpaper.";
            WallpaperSetter::instance()->setWallpaper(value);
        }
        else {
            m_cmdParser.showHelp();
        }

        return false;
    }
}

DeepinImageViewerDBus::DeepinImageViewerDBus(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{

}

DeepinImageViewerDBus::~DeepinImageViewerDBus()
{

}

void DeepinImageViewerDBus::backToMainWindow() const
{
    emit SignalManager::instance()->backToMainWindow();
}

void DeepinImageViewerDBus::enterAlbum(const QString &album)
{
    qDebug() << "Enter the album: " << album;
    // TODO
}

void DeepinImageViewerDBus::searchImage(const QString &keyWord)
{
    qDebug() << "Go to search view and search image by: " << keyWord;
    // TODO
}

void DeepinImageViewerDBus::editImage(const QString &path)
{
    qDebug() << "Go to edit view and begin editing: " << path;
    emit SignalManager::instance()->editImage(path);
}
