HEADERS += \
    $$PWD/signalmanager.h \
    $$PWD/wallpapersetter.h \
    $$PWD/commandline.h \
    $$PWD/configsetter.h \
    $$PWD/globaleventfilter.h \
    $$PWD/viewerthememanager.h \
    $$PWD/dbusclient.h

SOURCES += \
    $$PWD/signalmanager.cpp \
    $$PWD/wallpapersetter.cpp \
    $$PWD/commandline.cpp \
    $$PWD/configsetter.cpp \
    $$PWD/globaleventfilter.cpp \
    $$PWD/viewerthememanager.cpp \
    $$PWD/dbusclient.cpp

!isEmpty(FULL_FUNCTIONALITY) {
    HEADERS += $$PWD/importer.h \
               $$PWD/divdbuscontroller.h \
               $$PWD/dbmanager.h

    SOURCES += $$PWD/importer.cpp \
               $$PWD/divdbuscontroller.cpp \
               $$PWD/dbmanager.cpp
}
