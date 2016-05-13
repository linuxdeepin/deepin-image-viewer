#include "commandline.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include <QCommandLineOption>
#include <QApplication>
#include <QDebug>


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
    else {
        QMetaObject::invokeMethod(this, "processOption", Qt::QueuedConnection);
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

void CommandLine::processOption()
{
    SignalManager *sm = SignalManager::instance();
    QStringList names = m_cmdParser.optionNames();
    if (names.isEmpty()) {
        emit sm->backToMainWindow();
        return;
    }
    QString name = names.first();
    QString value = m_cmdParser.value(names.first());

    if (name == "o" || name == "open") {
        qDebug() << "Open image file: " << value;
        emit sm->viewImage(value);
    }
    else if (name == "a" || name == "album") {
        qDebug() << "Enter th album: " << value;
        // TODO
    }
    else if (name == "s" || name == "search") {
        qDebug() << "Go to search view and search image by: " << value;
        // TODO
    }
    else if (name == "e" || name == "edit") {
        qDebug() << "Go to edit view and begin editing: " << value;
        emit sm->editImage(value);
    }
    else if (name == "w" || name == "wallpaper") {
        qDebug() << "Set " << value << " as wallpaper.";
        WallpaperSetter::instance()->setWallpaper(value);
        qApp->quit();
    }
    else {
        m_cmdParser.showHelp();
    }
}
