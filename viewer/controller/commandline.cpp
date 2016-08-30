#include "commandline.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "controller/divdbuscontroller.h"
#include "frame/mainwindow.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
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
    {"l", "view", "View images.", "path1,path2,..."},
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
    m_cmdParser.addHelpOption();
    m_cmdParser.addVersionOption();
    m_cmdParser.addPositionalArgument("value", QCoreApplication::translate(
        "main", "Value that use for options."), "[value]");

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
    DeepinImageViewerDBus *dd = new DeepinImageViewerDBus(dApp->signalM);
    Q_UNUSED(dd);

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
        if (!names.isEmpty()) {
            name = names.first();
            value = m_cmdParser.value(name);
        }
        else if (!pas.isEmpty() && isImageSupported(pas.first())){
            name = "o";
            value = pas.first();
        }
        bool support = isImageSupported(value);


        if (name == "o" || name == "open") {
            qDebug() << "Open image file: " << value;
            if (isImageSupported(value)) {
                viewImage(value, QStringList());
                return true;
            }
            else {
                return false;
            }
        }
        else if (name == "view") {
            QStringList paths = value.split(",");
            for (QString path : paths) {
                if (! isImageSupported(path))
                    paths.removeAll(path);
            }
            if (! paths.isEmpty()) {
                viewImage(paths.first(), paths);
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
            dc->editImage(value);
        }
        else if ((name == "w" || name == "wallpaper") && support) {
            qDebug() << "Set " << value << " as wallpaper.";
            dApp->wpSetter->setWallpaper(value);
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
    emit dApp->signalM->backToMainPanel();
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
    emit dApp->signalM->editImage(path);
}
