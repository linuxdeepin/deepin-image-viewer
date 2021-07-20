#-------------------------------------------------
#
# Project created by QtCreator 2021-07-20T13:38:06
#
#-------------------------------------------------

QT       += widgets

TARGET = image-viewer
TEMPLATE = lib

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkcore
DEFINES += IMAGEVIEWER_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        imageviewer.cpp

HEADERS += \
        imageviewer.h \
        image-viewer_global.h 
DESTDIR = $$PWD/../../out/
unix {
    target.path = /usr/lib/
    INSTALLS += target
}
