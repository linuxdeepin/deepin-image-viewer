#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T14:34:59
#
#-------------------------------------------------

QT       += core gui sql concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle
#include (cutelogger/cutelogger.pri)
CONFIG += c++11 link_pkgconfig
PKGCONFIG +=  dtkwidget dtkutil dtkbase libexif
#gtk+-2.0
TARGET = deepin-image-viewer
TEMPLATE = app
INCLUDEPATH += utils

SOURCES += main.cpp \
    frame/blureframe.cpp \
    frame/mainwidget.cpp \
    frame/toptoolbar.cpp \
    frame/bottomtoolbar.cpp \
    frame/extensionpanel.cpp \
    controller/signalmanager.cpp \
    controller/databasemanager.cpp \
    module/importandexport/importer.cpp \
    module/album/albumpanel.cpp \
    module/timeline/timelinepanel.cpp \
    module/timeline/timelineviewframe.cpp \
    module/timeline/timelineimageview.cpp \
    widgets/thumbnaillistview.cpp \
    utils/imgutil.cpp \
    utils/libjpeg/jpeg-marker.c \
    utils/libjpeg/jpeg-data.c \
    module/timeline/scaleslider.cpp \
    module/timeline/sliderframe.cpp \
    widgets/tooltip.cpp \
    module/timeline/toptimelinetips.cpp \
    widgets/popupmenustyle.cpp \
    controller/popupmenumanager.cpp

HEADERS += \
    frame/blureframe.h \
    frame/mainwidget.h \
    frame/toptoolbar.h \
    frame/bottomtoolbar.h \
    frame/extensionpanel.h \
    controller/signalmanager.h \
    controller/databasemanager.h \
    module/importandexport/importer.h \
    module/album/albumpanel.h \
    module/timeline/timelinepanel.h \
    module/modulepanel.h \
    module/timeline/timelineviewframe.h \
    module/timeline/timelineimageview.h \
    widgets/thumbnaillistview.h \
    utils/imgutil.h \
    utils/libjpeg/jpeg-marker.h \
    utils/libjpeg/jpeg-data.h \
    module/timeline/scaleslider.h \
    module/timeline/sliderframe.h \
    widgets/tooltip.h \
    module/timeline/toptimelinetips.h \
    widgets/popupmenustyle.h \
    controller/popupmenumanager.h

#view
SOURCES += module/view/viewpanel.cpp \
        module/view/imageinfowidget.cpp \
    module/view/navigationwidget.cpp \
        module/view/imagewidget.cpp

HEADERS += module/view/viewpanel.h \
        module/view/imageinfowidget.h \
    module/view/navigationwidget.h \
        module/view/imagewidget.h
#edit
SOURCES += \
    module/edit/Cut.cpp \
    module/edit/FilterSetup.cpp \
    module/edit/FiltersPreview.cpp \
    module/edit/EditPanel.cpp \
    module/edit/filters/Filters.cpp \
    module/edit/filters/RationalColorTransform.cpp \
    module/edit/filters/ColorTransforms.cpp \
    module/edit/filters/Filter2D.cpp \
    module/edit/filters/FilterObj.cpp

HEADERS += \
    module/edit/Cut.h \
    module/edit/FilterSetup.h \
    module/edit/FiltersPreview.h \
    module/edit/EditPanel.h \
    module/edit/filters/Filter2D.h \
    module/edit/filters/ConvolutionSampler.h \
    module/edit/filters/Filters.h \
    module/edit/filters/Samplers.h \
    module/edit/filters/RationalColorTransform.h \
    module/edit/filters/ColorTransforms.h \
    module/edit/filters/FilterObj.h \
    module/edit/filters/FilterId.h

RESOURCES += \
    resources.qrc
