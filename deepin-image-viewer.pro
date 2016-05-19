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

include (frame/frame.pri)
include (module/modules.pri)
include (widgets/widgets.pri)
include (utils/utils.pri)
include (controller/controller.pri)

SOURCES += main.cpp

PREFIX = /usr
binary.path = $${PREFIX}/bin
binary.files = deepin-image-viewer

desktop.path = $${PREFIX}/share/applications/
desktop.files =  deepin-image-viewer.desktop

icons.path = $${PREFIX}/share/deepin-image-viewer/icons
icons.files = resources/images/*

dbus_service.files += com.deepin.deepinimageviewer.service
dbus_service.path = /usr/share/dbus-1/services

INSTALLS = binary desktop icons dbus_service

RESOURCES += \
    resources.qrc
