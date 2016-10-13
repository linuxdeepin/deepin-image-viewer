#extralib.target = extra
#extralib.commands = $$PWD/prebuild;
#extralib.depends =
#QMAKE_EXTRA_TARGETS += extralib
#PRE_TARGETDEPS = extra

QMAKE_CXXFLAGS += $$system(env PKG_CONFIG_PATH=$$PWD/lib/pkgconfig pkg-config --cflags dtkwidget)
LIBS +=  $$system(env PKG_CONFIG_PATH=$$PWD/lib/pkgconfig pkg-config --libs dtkwidget)
QMAKE_CXXFLAGS += $$system(env PKG_CONFIG_PATH=$$PWD/lib/pkgconfig pkg-config --cflags dtkutil)
LIBS +=  $$system(env PKG_CONFIG_PATH=$$PWD/lib/pkgconfig pkg-config --libs dtkutil)
QMAKE_CXXFLAGS += $$system(env PKG_CONFIG_PATH=$$PWD/lib/pkgconfig pkg-config --cflags dtkbase)
LIBS +=  $$system(env PKG_CONFIG_PATH=$$PWD/lib/pkgconfig pkg-config --libs dtkbase)

QT += network platformsupport-private
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += link_pkgconfig
PKGCONFIG += xcb-util libstartup-notification-1.0
