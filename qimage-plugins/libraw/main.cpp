/*
 * Copyright (C) 2012 Alberto Mardegan <info@mardy.it>
 *
 * This file is part of QtRaw.
 *
 * QtRaw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QtRaw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtRaw.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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

#include <QByteArray>
#include <QImageIOHandler>
#include <QIODevice>
#include <QStringList>

#include "rawiohandler.h"

class FreeimageQt5Plugin: public QImageIOPlugin
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "raw.json")
#endif

public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device,
                              const QByteArray &format) const Q_DECL_OVERRIDE;
    QImageIOHandler *create(QIODevice *device,
                            const QByteArray &format = QByteArray()) const Q_DECL_OVERRIDE;
};

QStringList FreeimageQt5Plugin::keys() const
{

    const QStringList raws = QStringList()
                             << QLatin1String("CR2") << QLatin1String("CRW")   // Canon cameras
                             << QLatin1String("DCR") << QLatin1String("KDC")   // Kodak cameras
                             << QLatin1String("MRW")            // Minolta cameras
                             << QLatin1String("NEF")            // Nikon cameras
                             << QLatin1String("ORF")            // Olympus cameras
                             << QLatin1String("PEF")            // Pentax cameras
                             << QLatin1String("RAF")            // Fuji cameras
                             << QLatin1String("SRF")            // Sony cameras
                             << QLatin1String("DNG");           //
//            << QLatin1String("X3F");           // Sigma cameras
    return raws;
}

QImageIOPlugin::Capabilities
FreeimageQt5Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (keys().contains(format.toUpper()) ||
            format == "tif" ||
            format == "tiff")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return nullptr;

    Capabilities cap;
    if (device->isReadable() && RawIOHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *FreeimageQt5Plugin::create(QIODevice *device,
                                            const QByteArray &format) const
{
    RawIOHandler *handler = new RawIOHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(qtraw, RawPlugin)
#endif

#include "main.moc"
