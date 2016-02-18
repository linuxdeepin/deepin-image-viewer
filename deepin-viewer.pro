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


SOURCES += main.cpp\
        mainwidget.cpp

HEADERS  += mainwidget.h
