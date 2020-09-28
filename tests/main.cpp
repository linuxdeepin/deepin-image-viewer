#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <DLog>
#include <QTranslator>
#include <DApplicationSettings>
#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"
#include <QTimer>
using namespace Dtk::Core;
int main(int argc, char *argv[])
{

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Application::loadDXcbPlugin();
    Application a(argc,argv);
    a.setAttribute(Qt::AA_ForceRasterWidgets);
    testing::InitGoogleTest();
    if (!service::isDefaultImageViewer()) {
        qDebug() << "Set defaultImage viewer succeed:" << service::setDefaultImageViewer(true);
    } else {
        qDebug() << "Deepin Image Viewer is defaultImage!";
    }
    RUN_ALL_TESTS();

    return a.exec();
//  return  RUN_ALL_TESTS();
//return a.exec();

}
