#ifndef DEEPINIMAGEVIEWERDBUS_H
#define DEEPINIMAGEVIEWERDBUS_H

#include <QObject>
#include "controller/signalmanager.h"
#include <QDBusAbstractAdaptor>

class DeepinImageViewerDBus: public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.DeepinImageViewer")
    Q_CLASSINFO("D-Bus Introspection", ""
"<interface name=\"com.deepin.DeepinImageViewer\">\n"
    "<method name=\"backToMainWindow\"/>\n"
    "<method name=\"activeWindow\"/>\n"
    "<method name=\"enterAlbum\">\n"
    "   <arg direction=\"in\" type=\"s\"/>\n"
    "</method>\n"
    "<method name=\"searchImage\">\n"
    "   <arg direction=\"in\" type=\"s\"/>\n"
    "</method>\n"
    "<method name=\"editImage\">\n"
    "   <arg direction=\"in\" type=\"s\"/>\n"
    "</method>\n"
 "</interface>\n")
public:
    explicit DeepinImageViewerDBus(SignalManager* parent);
    ~DeepinImageViewerDBus();

    inline SignalManager *parent() const
    { return static_cast<SignalManager *>(QObject::parent()); }

    Q_SLOT void backToMainWindow() const;
    Q_SLOT void activeWindow();
    Q_SLOT void enterAlbum(const QString &album);
    Q_SLOT void searchImage(const QString &keyWord);
    Q_SLOT void editImage(const QString &path);
};
#endif // DEEPINIMAGEVIEWERDBUS_H
