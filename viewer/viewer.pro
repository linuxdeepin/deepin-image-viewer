#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------

QT += core gui sql dbus concurrent svg  printsupport
# QT += x11extras
qtHaveModule(opengl): QT += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle
CONFIG += c++11 link_pkgconfig
PKGCONFIG +=   libexif dtkwidget  gio-qt udisks2-qt5
# PKGCONFIG += xext x11 gio-unix-2.0
 QT += dtkwidget
 QT += dbus
LIBS += -lfreeimage

#gtk+-2.0
TARGET = deepin-image-viewer
TEMPLATE = app
INCLUDEPATH += utils

isEmpty(FULL_FUNCTIONALITY) {
    DEFINES += LITE_DIV
    DEFINES += USE_UNIONIMAGE
}

isEmpty(PREFIX){
    PREFIX = /usr
}

include (frame/frame.pri)
include (module/modules.pri)
include (widgets/widgets.pri)
include (utils/utils.pri)
include (controller/controller.pri)
include (service/service.pri)

!isEmpty(FULL_FUNCTIONALITY) {
    include (settings/settings.pri)
    include (dirwatcher/dirwatcher.pri)
}

HEADERS += \
    application.h

SOURCES += main.cpp \
    application.cpp

RESOURCES += \
    resources.qrc \
    icons/theme-icons.qrc

# Automating generation .qm files from .ts files
#!system($$PWD/generate_translations.sh): error("Failed to generate translation")

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
}

#TRANSLATIONS += \
#    translations/deepin-image-viewer.ts\
#    translations/deepin-image-viewer_zh_CN.ts

BINDIR = $$PREFIX/bin
APPSHAREDIR = $$PREFIX/share/deepin-image-viewer
MANDIR = $$PREFIX/share/dman/deepin-image-viewer
MANICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
APPICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps

DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

target.path = $$BINDIR

desktop.path = $$PREFIX/share/applications/
desktop.files = $$PWD/deepin-image-viewer.desktop

icons.path = $$APPSHAREDIR/icons
icons.files = $$PWD/assets/images/*

manual.path = $$MANDIR
manual.files = $$PWD/docs/doc/*

manual_icon.path = $$MANICONDIR
manual_icon.files = $$PWD/docs/doc/common/deepin-image-viewer.svg

app_icon.path = $$APPICONDIR
app_icon.files = $$PWD/assets/images/logo/deepin-image-viewer.svg

dbus_service.path =  $$PREFIX/share/dbus-1/services
dbus_service.files += $$PWD/com.deepin.ImageViewer.service

translations.path = $$APPSHAREDIR/translations
translations.files = $$PWD/translations/*.qm

INSTALLS = target desktop dbus_service icons manual manual_icon app_icon translations

DISTFILES += \
    com.deepin.ImageViewer.service

load(dtk_qmake)
host_sw_64: {
# 在 sw_64 平台上添加此参数，否则会在旋转图片时崩溃
    QMAKE_CFLAGS += -mieee
    QMAKE_CXXFLAGS += -mieee
}
