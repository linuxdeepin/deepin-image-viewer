#ifndef APPLICATION_H
#define APPLICATION_H

#include <DApplication>
DWIDGET_USE_NAMESPACE
using namespace Dtk::Core;

#define dApp (static_cast<Application *>(QCoreApplication::instance()))
class Application : public DApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    ~Application();
signals:
    void sigQuit();
};

#endif // APPLICATION_H
