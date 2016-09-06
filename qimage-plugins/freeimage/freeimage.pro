TARGET = xfreeimage
TEMPLATE = lib
CONFIG += \
        c++11 \
	link_pkgconfig \
	plugin

DESTDIR = imageformats
LIBS += -L/usr/lib/x86_64-linux-gnu -lfreeimage

HEADERS += \
    freeimagehandler.h

SOURCES += \
    freeimagehandler.cpp \
    main.cpp

OTHER_FILES += \
	freeimage.json

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
