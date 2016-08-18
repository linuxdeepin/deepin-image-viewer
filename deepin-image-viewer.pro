#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------

QT       += core gui sql dbus concurrent svg x11extras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle
CONFIG += c++11 link_pkgconfig
PKGCONFIG +=  dtkwidget dtkutil dtkbase libexif x11 xext
#gtk+-2.0
TARGET = deepin-image-viewer
TEMPLATE = app
INCLUDEPATH += utils

isEmpty(PREFIX){
    PREFIX = /usr
}

include (frame/frame.pri)
include (module/modules.pri)
include (widgets/widgets.pri)
include (utils/utils.pri)
include (controller/controller.pri)
include (service/service.pri)

SOURCES += main.cpp \
    application.cpp

BINDIR = $$PREFIX/bin
APPSHAREDIR = $$PREFIX/share/deepin-image-viewer
MANDIR = $$PREFIX/share/dman/deepin-image-viewer
MANICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps

DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

binary.path = $$BINDIR
binary.files = deepin-image-viewer

desktop.path = $${PREFIX}/share/applications/
desktop.files =  deepin-image-viewer.desktop

icons.path = $$APPSHAREDIR/icons
icons.files = resources/images/*

manual.path = $$MANDIR
manual.files = doc/*
manual_icon.path = $$MANICONDIR
manual_icon.files = doc/common/viewer.svg

dbus_service.files += com.deepin.deepinimageviewer.service
dbus_service.path = /usr/share/dbus-1/services

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    system($$PWD/generate_translations.sh)
}

translations.path = $$APPSHAREDIR/translations
translations.files = translations/*.qm

INSTALLS = binary desktop dbus_service icons manual manual_icon translations

RESOURCES += \
    resources.qrc

HEADERS += \
    application.h
