
include (slideeffect/slideeffect.pri)

HEADERS += \
    $$PWD/imageinfowidget.h \
    $$PWD/imagewidget.h \
    $$PWD/navigationwidget.h \
    $$PWD/viewpanel.h \
    $$PWD/contents/tbcontent.h \
    $$PWD/contents/ttlcontent.h \
    $$PWD/contents/ttmcontent.h \
    $$PWD/imagesliderframe.h

SOURCES += \
    $$PWD/imageinfowidget.cpp \
    $$PWD/imagewidget.cpp \
    $$PWD/navigationwidget.cpp \
    $$PWD/viewpanel.cpp \
    $$PWD/contents/tbcontent.cpp \
    $$PWD/contents/ttlcontent.cpp \
    $$PWD/contents/ttmcontent.cpp \
    $$PWD/imagesliderframe.cpp

RESOURCES += \
    $$PWD/view.qrc
