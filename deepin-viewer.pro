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

TARGET = deepin-viewer
TEMPLATE = app


SOURCES += main.cpp \
    frame/blureframe.cpp \
    frame/mainwidget.cpp \
    frame/toptoolbar.cpp \
    frame/bottomtoolbar.cpp \
    signalmanager.cpp \
    frame/extensionpanel.cpp

HEADERS += \
    frame/blureframe.h \
    frame/mainwidget.h \
    frame/toptoolbar.h \
    frame/bottomtoolbar.h \
    signalmanager.h \
    frame/extensionpanel.h



RESOURCES += \
    resources.qrc
