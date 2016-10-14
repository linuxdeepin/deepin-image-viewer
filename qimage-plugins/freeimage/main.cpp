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

    // DO NOT handle the types which already supported by qt
    // FIXME: And there is a bug with FIF_PNG handling QIcon::addFile
    QList<FREE_IMAGE_FORMAT> fifs;
    fifs << FIF_PNG << FIF_JP2 << FIF_JPEG << FIF_BMP << FIF_GIF << FIF_PBM
         << FIF_PGM << FIF_PPM << FIF_XBM << FIF_XPM;
    // And, RAW formats render incorrect by freeimage
    const QStringList raws = QStringList()
            << "CR2" << "CRW"   // Canon cameras
            << "DCR" << "KDC"   // Kodak cameras
            << "MRW"            // Minolta cameras
            << "NEF"            // Nikon cameras
            << "ORF"            // Olympus cameras
            << "PEF"            // Pentax cameras
            << "RAF"            // Fuji cameras
            << "SRF"            // Sony cameras
            << "X3F";           // Sigma cameras

    const bool supportedFormat = (raws.indexOf(format.toUpper()) == -1
                                && (fifs.indexOf(fif) == -1)
                                && FreeImage_FIFSupportsReading(fif));

    if (! (device->openMode() & QIODevice::WriteOnly)
            && device
            && device->isReadable()
            && supportedFormat) {
        return Capabilities(CanRead);
    }
    else {
        return 0;
    }
}

QImageIOHandler *
FreeimageQt5Plugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new FreeImageHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "main.moc"
