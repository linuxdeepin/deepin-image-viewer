#include "dbusclient.h"




Dbusclient::Dbusclient(QObject *parent)
    : QDBusAbstractInterface(staticInterfaceService(), staticInterfacePath(),
                             staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{

   // QDBusConnection::sessionBus().connect(staticInterfaceService(),staticInterfacePath(),staticInterfaceName(),"com.deepin.Draw",  "com.deepin.Draw",this,SLOT(propertyChanged(QDBusMessage)));

    connect(dApp->signalM,&SignalManager::sigDrawingBoard,this,&Dbusclient::openDrawingBoard);
}

Dbusclient::~Dbusclient()
{

}

//void Dbusclient::propertyChanged(const QDBusMessage &msg)
//{
//    Q_UNUSED(msg)
//}


void Dbusclient::openDrawingBoard(const QStringList& paths)
{
    QList<QString> list=paths;

    openFiles(list);
}

