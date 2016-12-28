#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"

#include <QApplication>
#include <DLog>
#include <QTranslator>

using namespace Dtk::Util;

namespace {

const char kPlatformThemeName[] = "QT_QPA_PLATFORMTHEME";

}  // namespace

int main(int argc, char *argv[])
{
#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
#endif
    // If platform theme name is empty, fallback to gtk2.
    // gtk2 theme is included in libqt5libqgtk2 package.

    //TODO: the Qt default theme's name is empty.
    //so i comment out the code.
    // if (qgetenv(kPlatformThemeName).length() == 0) {
    //        qDebug() << qgetenv(kPlatformThemeName);
    //      qputenv(kPlatformThemeName, "gtk2");
    //    }

    Application::loadDXcbPlugin();
    Application a(argc, argv);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    dInfo()<< "LogFile:" << DLogManager::getlogFilePath();

    if (!service::isDefaultImageViewer()) {
        service::setDefaultImageViewer(true);
    }

    CommandLine *cl = CommandLine::instance();

    if (cl->processOption()) {
        if (Application::isDXcbPlatform()) {
            quick_exit(a.exec());
        } else {
            return a.exec();
        }
    }
    else {
        return 0;
    }
}
