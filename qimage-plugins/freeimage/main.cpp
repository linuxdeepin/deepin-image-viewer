#include <QByteArray>
#include <QImageIOHandler>
#include <QIODevice>
#include <QStringList>

#include "freeimagehandler.h"

class FreeimageQt5Plugin: public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "freeimage.json")

public:
    Capabilities capabilities(QIODevice *device,
                              const QByteArray &format) const Q_DECL_OVERRIDE;
    QImageIOHandler *create(QIODevice *device,
        const QByteArray &format = QByteArray()) const Q_DECL_OVERRIDE;
};

QImageIOPlugin::Capabilities
FreeimageQt5Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    FREE_IMAGE_FORMAT fif = FreeImageHandler::GetFIF(device, format);
    if (! (device->openMode() & QIODevice::WriteOnly)
            && device
            && device->isReadable()
            && FreeImage_FIFSupportsReading(fif)) {
        return Capabilities(CanRead);
    }
    return 0;
}

QImageIOHandler *FreeimageQt5Plugin::create(QIODevice *device,
                                   const QByteArray &format) const
{
    QImageIOHandler *handler = new FreeImageHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "main.moc"
