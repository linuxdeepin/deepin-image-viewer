#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------

QT += core gui sql dbus concurrent svg x11extras printsupport
qtHaveModule(opengl): QT += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle
CONFIG += c++11 link_pkgconfig
PKGCONFIG += x11 xext libexif dtkwidget
LIBS += -lfreeimage
#gtk+-2.0
TARGET = deepin-image-viewer
TEMPLATE = app
INCLUDEPATH += utils

DEFINES += STATIC_LIB

isEmpty(PREFIX){
    PREFIX = /usr
}

include (frame/frame.pri)
include (module/modules.pri)
include (widgets/widgets.pri)
include (utils/utils.pri)
include (controller/controller.pri)
include (service/service.pri)
include (settings/settings.pri)
include (dirwatcher/dirwatcher.pri)

HEADERS += \
    application.h

SOURCES += main.cpp \
    application.cpp

RESOURCES += \
    resources.qrc

# Automating generation .qm files from .ts files
!system($$PWD/generate_translations.sh): error("Failed to generate translation")

TRANSLATIONS += \
    translations/deepin-image-viewer.ts\
    translations/deepin-image-viewer_zh_CN.ts

BINDIR = $$PREFIX/bin
APPSHAREDIR = $$PREFIX/share/deepin-image-viewer
MANDIR = $$PREFIX/share/dman/deepin-image-viewer
MANICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
APPICONDIR = $$PREFIX/share/icons/deepin/apps/scalable

DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

target.path = $$BINDIR

desktop.path = $$PREFIX/share/applications/
desktop.files = $$PWD/deepin-image-viewer.desktop

icons.path = $$APPSHAREDIR/icons
icons.files = $$PWD/resources/images/*

manual.path = $$MANDIR
manual.files = $$PWD/doc/*

manual_icon.path = $$MANICONDIR
manual_icon.files = $$PWD/doc/common/deepin-image-viewer.svg

app_icon.path = $$APPICONDIR
app_icon.files = $$PWD/resources/images/logo/deepin-image-viewer.svg

dbus_service.path =  $$PREFIX/share/dbus-1/services
dbus_service.files += $$PWD/com.deepin.ImageViewer.service

translations.path = $$APPSHAREDIR/translations
translations.files = $$PWD/translations/*.qm

INSTALLS = target desktop dbus_service icons manual manual_icon app_icon translations

DISTFILES += \
    com.deepin.ImageViewer.service
