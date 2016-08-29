QT += core gui widgets
CONFIG += c++11 link_pkgconfig

TARGET = test
TEMPLATE = app

isEmpty(PREFIX){
    PREFIX = /usr
}

HEADERS += \
    mainwidget.h

SOURCES += \
    mainwidget.cpp \
    main.cpp


