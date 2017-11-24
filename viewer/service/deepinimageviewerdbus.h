/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

public Q_SLOTS:
     Q_SCRIPTABLE void backToMainWindow() const;
     Q_SCRIPTABLE void activeWindow();
     Q_SCRIPTABLE void enterAlbum(const QString &album);
     Q_SCRIPTABLE void searchImage(const QString &keyWord);
     Q_SCRIPTABLE void editImage(const QString &path);
};
#endif // DEEPINIMAGEVIEWERDBUS_H
