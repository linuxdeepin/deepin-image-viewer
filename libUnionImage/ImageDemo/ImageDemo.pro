# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = ImageDemo

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageDemo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11
QMAKE_LFLAGS += -ldl

HEADERS += \
        mainwindow.h \
    imagedemothread.h \
    moviegraphicsitem.h

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imagedemothread.cpp \
    moviegraphicsitem.cpp


FORMS += \
    mainwindow.ui



CONFIG(debug ,debug | release){
    message("UnionImage Library Demo debug")
    LIBS += -L$$PWD/../bin_Debug/ -lUnionImage
}

CONFIG(release ,debug | release){
    message("UnionImage Library Demo release")
    LIBS += -L$$PWD/../bin_Release/ -lUnionImage
}

INCLUDEPATH += $$PWD/../UnionImage
DEPENDPATH += $$PWD/../UnionImage
