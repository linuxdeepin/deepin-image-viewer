#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <DApplication>

class Application;
class ConfigSetter;
class DatabaseManager;
class DBManager;
class Exporter;
class Importer;
class SignalManager;
class WallpaperSetter;

#if defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<Application *>(QCoreApplication::instance()))

DWIDGET_USE_NAMESPACE

class Application : public DApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);

    ConfigSetter *setter = nullptr;
    DBManager *dbM = nullptr;
    Exporter *exporter = nullptr;
    Importer *importer = nullptr;
    SignalManager *signalM = nullptr;
    WallpaperSetter *wpSetter = nullptr;

private:
    void initChildren();
    void initI18n();
};

#endif  // APPLICATION_H_
