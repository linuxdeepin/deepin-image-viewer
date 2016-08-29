#ifndef FREEIMAGEHANDLER_H
#define FREEIMAGEHANDLER_H

#include <FreeImage.h>
#include <QImageIOHandler>
#include <QImage>
#include <QRgb>

class FreeImageHandler : public QImageIOHandler
{
public:
    FreeImageHandler();
    virtual ~FreeImageHandler();

    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;

    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

public:
    static FreeImageIO &fiio();
    static FREE_IMAGE_FORMAT GetFIF(QIODevice *device, const QByteArray &fmt);

private:
    static QImage& noneQImage();
    static bool isNoneQImage(const QImage& qi);
    static QVector<QRgb>& nonePalette();
    static bool isNonePalette(const QVector<QRgb> &pal);
    static QImage FIBitmapToQImage(FIBITMAP *dib);
    static QVector<QRgb> getPalette(FIBITMAP *dib);
};

#endif // FREEIMAGEHANDLER_H
