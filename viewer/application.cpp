#include "application.h"

#include "controller/configsetter.h"
#include "controller/databasemanager.h"
#include "controller/exporter.h"
#include "controller/globaleventfilter.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"

#include <QDebug>
#include <QTranslator>

namespace {

}  // namespace

Application::Application(int& argc, char** argv)
    : DApplication(argc, argv)
{
    initI18n();
    setOrganizationName("deepin");
    setApplicationName("deepin-image-viewer");
    setApplicationDisplayName(tr("Deepin Image Viewer"));
    setApplicationVersion("1.1");

    installEventFilter(new GlobalEventFilter());


    initChildren();
}

void Application::initChildren()
{
    setter = ConfigSetter::instance();
    databaseM = DatabaseManager::instance();
    exporter = Exporter::instance();
    importer = Importer::instance();
    signalM = SignalManager::instance();
    wpSetter = WallpaperSetter::instance();
}

void Application::initI18n()
{
    // install translators
    QTranslator *translator = new QTranslator;
    translator->load(APPSHAREDIR"/translations/deepin-image-viewer_"
                     + QLocale::system().name() + ".qm");
    installTranslator(translator);

}
