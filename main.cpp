#include "frame/mainwidget.h"
#include <QApplication>
#include <DLog>

using namespace Dtk::Util;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-viewer");
    a.setApplicationDisplayName("Deepin Viewer");

//    // install translators
//    QTranslator translator;
//    translator.load("/usr/share/deepin-viewer/translations/deepin-viewer_" + QLocale::system().name());
//    a.installTranslator(&translator);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    dInfo()<< "LogFile:" << DLogManager::getlogFilePath();

    MainWidget w;
    w.show();

    return a.exec();
}
