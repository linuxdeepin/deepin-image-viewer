#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------
#system($$PWD/vendor/prebuild)

QT += core gui sql dbus concurrent svg x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle
CONFIG += c++11 link_pkgconfig
PKGCONFIG += x11 xext dtkwidget dtkutil dtkbase
LIBS += -L/usr/lib/x86_64-linux-gnu -lfreeimage
#gtk+-2.0
TARGET = deepin-image-viewer
TEMPLATE = app
INCLUDEPATH += utils

#DEFINES += STATIC_LIB

isEmpty(PREFIX){
    PREFIX = /usr
}

include (frame/frame.pri)
include (module/modules.pri)
include (widgets/widgets.pri)
include (utils/utils.pri)
include (controller/controller.pri)
include (service/service.pri)

HEADERS += \
    application.h

SOURCES += main.cpp \
    application.cpp

RESOURCES += \
    resources.qrc

BINDIR = $$PREFIX/bin
APPSHAREDIR = $$PREFIX/share/deepin-image-viewer
MANDIR = $$PREFIX/share/dman/deepin-image-viewer
MANICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
APPICONDIR = $$PREFIX/share/icons/deepin/apps/scalable

DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

target.path = $$BINDIR

desktop.path = $${PREFIX}/share/applications/
desktop.files =  deepin-image-viewer.desktop

icons.path = $$APPSHAREDIR/icons
icons.files = resources/images/*

manual.path = $$MANDIR
manual.files = doc/*
manual_icon.path = $$MANICONDIR
manual_icon.files = doc/common/deepin-image-viewer.svg
app_icon.path = $$APPICONDIR
app_icon.files = resources/images/logo/deepin-image-viewer.svg

dbus_service.files += com.deepin.deepinimageviewer.service
dbus_service.path = /usr/share/dbus-1/services

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    system($$PWD/generate_translations.sh)
}

translations.path = $$APPSHAREDIR/translations
translations.files = translations/*.qm

INSTALLS = target desktop dbus_service icons manual manual_icon app_icon translations
