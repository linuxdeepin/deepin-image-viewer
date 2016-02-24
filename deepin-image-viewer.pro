#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#include (cutelogger/cutelogger.pri)
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkutil dtkbase

TARGET = deepin-image-viewer
TEMPLATE = app


SOURCES += main.cpp \
    frame/blureframe.cpp \
    frame/mainwidget.cpp \
    frame/toptoolbar.cpp \
    frame/bottomtoolbar.cpp \
    frame/extensionpanel.cpp \
    controller/signalmanager.cpp

HEADERS += \
    frame/blureframe.h \
    frame/mainwidget.h \
    frame/toptoolbar.h \
    frame/bottomtoolbar.h \
    frame/extensionpanel.h \
    controller/signalmanager.h



RESOURCES += \
    resources.qrc
