#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <QApplication>

class Application;
class ConfigSetter;
class DatabaseManager;
class Exporter;
class Importer;
class SignalManager;
class WallpaperSetter;

#if defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<Application *>(QCoreApplication::instance()))

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application();

    ConfigSetter *setter = nullptr;
    DatabaseManager *databaseM = nullptr;
    Exporter *exporter = nullptr;
    Importer *importer = nullptr;
    SignalManager *signalM = nullptr;
    WallpaperSetter *wpSetter = nullptr;

private:
    void initChildren();
    void initI18n();
};

#endif  // APPLICATION_H_
