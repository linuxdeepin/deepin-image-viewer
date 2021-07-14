#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------

QT += core gui dbus concurrent svg  printsupport sql
# QT += x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle
CONFIG += c++11 link_pkgconfig
PKGCONFIG +=   libexif dtkwidget  gio-qt
# PKGCONFIG += xext x11 gio-unix-2.0
 QT += dtkwidget
 QT += dbus
#CONFIG += object_parallel_to_source
LIBS += -lfreeimage

#gtk+-2.0
TARGET = deepin-image-viewer
TEMPLATE = app
INCLUDEPATH += src/utils
INCLUDEPATH += src

isEmpty(FULL_FUNCTIONALITY) {
    DEFINES += LITE_DIV
    DEFINES += USE_UNIONIMAGE
}

isEmpty(PREFIX){
    PREFIX = /usr
}

include (src/frame/frame.pri)
include (src/module/modules.pri)
include (src/widgets/widgets.pri)
include (src/utils/utils.pri)
include (src/controller/controller.pri)
include (src/service/service.pri)
include (src/third-party/accessibility/accessibility-suite.pri)

!isEmpty(FULL_FUNCTIONALITY) {
    include (src/settings/settings.pri)
    include (src/dirwatcher/dirwatcher.pri)
}

HEADERS += \
    src/application.h \
    src/accessibility/acobjectlist.h \
    src/accessibility/ac-desktop-define.h

SOURCES += src/main.cpp \
    src/application.cpp

RESOURCES += \
    assets/images/resources.qrc \
    assets/icons/theme-icons.qrc

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

icons.path = $$APPSHAREDIR/assets/icons
icons.files = $$PWD/assets/images/*

deepin_manual.path =/usr/share/deepin-manual/manual-assets/application/
deepin_manual.files=$$PWD/assets/deepin-image-viewer

manual.path = $$MANDIR
manual.files = $$PWD/assets/doc/*

manual_icon.path = $$MANICONDIR
manual_icon.files = $$PWD/assets/doc/common/deepin-image-viewer.svg

app_icon.path = $$APPICONDIR
app_icon.files = $$PWD/assets/images/logo/deepin-image-viewer.svg

dbus_service.path =  $$PREFIX/share/dbus-1/services
dbus_service.files += $$PWD/com.deepin.ImageViewer.service

translations.path = $$APPSHAREDIR/translations
translations.files = $$PWD/translations/*.qm

INSTALLS = target desktop dbus_service icons manual manual_icon app_icon translations deepin_manual

DISTFILES += \
    com.deepin.ImageViewer.service

load(dtk_qmake)
QMAKE_CXXFLAGS += -Wl,-as-need -fPIE
QMAKE_CFLAGS += -Wl,-as-need -Wl,-E -fPIE
QMAKE_LFLAGS+=-Wl,-as-need -Wl,-E -pie

host_sw_64: {
# 在 sw_64 平台上添加此参数，否则会在旋转图片时崩溃
    QMAKE_CFLAGS += -mieee
    QMAKE_CXXFLAGS += -mieee
}

host_mips64:{
   QMAKE_CXX += -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi
   QMAKE_CXXFLAGS += -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi
}
