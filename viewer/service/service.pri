HEADERS += \
    $$PWD/defaultimageviewer.h \
    $$PWD/third-party/simpleini/ConvertUTF.h \
    $$PWD/third-party/simpleini/SimpleIni.h \
    $$PWD/deepinimageviewerdbus.h

SOURCES += \
    $$PWD/defaultimageviewer.cpp \
    $$PWD/third-party/simpleini/snippets.cpp \
    $$PWD/third-party/simpleini/ConvertUTF.c \
    $$PWD/deepinimageviewerdbus.cpp

DISTFILES += \
    $$PWD/third-party/simpleini/ini.syn \
    $$PWD/third-party/simpleini/LICENCE.txt \
    $$PWD/third-party/simpleini/CMakeLists.txt \
    $$PWD/third-party/simpleini/README.md
