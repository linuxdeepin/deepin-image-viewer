#include "freeimagehandler.h"

#include <QColor>
#include <QVariant>
#include <QDebug>

// FREEIMAGE IO PROCS /////////////////////////////////////////////////

// NOTE: this is unclear from FreeImage manual, but interface of FreeImageIO
// is modeled after fread, fwrite, fseek, ftell.

static  unsigned //__attribute__((stdcall))
ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
{
    return static_cast<QIODevice*>(handle)->read((char*)buffer,size*count);
}


static unsigned //__attribute__((stdcall))
WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
{
    QIODevice *quid = static_cast<QIODevice*>(handle);
    return quid->write((char*)buffer,size*count);
}


static  int //__attribute__((stdcall))
SeekProc(fi_handle handle, long offset, int origin)
{
    QIODevice *quid = static_cast<QIODevice*>(handle);

    if (quid->isOpen()) {
        switch (origin)
        {
        default:
        case SEEK_SET:
            return int(!quid->seek(offset));
        case SEEK_CUR:
            return int(!quid->seek(quid->pos()+offset));
        case SEEK_END:
            if (!quid->isSequential())
            {
                quint64 len = quid->bytesAvailable();
                return int(!quid->seek(len+offset));
            }
            break;
        }
    }
    return (-1);
}

static long //__attribute__((stdcall))
TellProc(fi_handle handle)
{
    return static_cast<QIODevice*>(handle)->pos();
}

// RAII WRAPPER FOR DIBS ////////////////////////////////////////////////
class ScopedDib
{
private:
    FIBITMAP *m_dib;

    ScopedDib(const ScopedDib&);
    ScopedDib& operator=(const ScopedDib&);
public:
    explicit ScopedDib(FIBITMAP *dib)
        : m_dib(dib)
    {
    }
    ~ScopedDib()
    {
        if (m_dib)
        {
            FreeImage_Unload(m_dib);
            m_dib = 0;
        }
    }
    FIBITMAP *get() const
    {
        return m_dib;
    }

    void reset(FIBITMAP *p = 0)
    {
        Q_ASSERT(p == 0 || p != m_dib);
        ScopedDib(p).swap(*this);
    }

    void swap(ScopedDib &rhs)
    {
        FIBITMAP *temp = rhs.m_dib;
        rhs.m_dib = m_dib;
        m_dib = temp;
    }

    // implicit conversion to bool a-la boost
    typedef FIBITMAP *ScopedDib::*uncpecified_bool_type;
    operator uncpecified_bool_type() const
    {
        return (m_dib == NULL)? NULL : &ScopedDib::m_dib;
    }
    bool operator!() const
    {
        return m_dib == NULL;
    }

};

FreeImageHandler::FreeImageHandler()
{

}

FreeImageHandler::~FreeImageHandler()
{

}

bool FreeImageHandler::canRead() const
{
    FREE_IMAGE_FORMAT fif = GetFIF(device(), format());
    return FreeImage_FIFSupportsReading(fif);
}

bool FreeImageHandler::read(QImage *image)
{
    FREE_IMAGE_FORMAT fif = GetFIF(device(), format());
    if (!FreeImage_FIFSupportsReading(fif)) {
        return false;
    }

    ScopedDib dib(0);

    // NOTE: FreeImage_LoadFromHandle has a lot of bugs,
    // some format can only load from memory, such as FIF_PSD and FIF_TIFF,
    // we will try FreeImage_LoadFromHandle later if load from memory failed

    // HACK: FreeImage(at least ver. 3.17.0) can not load FIF_PSD and FIF_TIFF
    // from stream. We load it from memory.
    QByteArray mem = device()->readAll();
    if (mem.isEmpty())
        return false;
    FIMEMORY *fmem = FreeImage_OpenMemory((BYTE*)mem.data(), mem.size());
    if (!fmem)
        return false;
    dib.reset(FreeImage_LoadFromMemory(fif, fmem));
    FreeImage_CloseMemory(fmem);


    if (! dib) {
        dib.reset(FreeImage_LoadFromHandle(fif, &fiio(), (fi_handle)device()));
        if (! dib) {
            qDebug() << "Can not load image's data from device()";
            return false;
        }
    }

    QImage result = FIBitmapToQImage(dib.get());

    if (isNoneQImage(result)) {
        qDebug() << "Convert FIBitmap to QImage failed! Format: " << fif;
        return false;
    }

    // set resolution
    result.setDotsPerMeterX(FreeImage_GetDotsPerMeterX(dib.get()));
    result.setDotsPerMeterY(FreeImage_GetDotsPerMeterY(dib.get()));

    // set palette
    if (FreeImage_GetPalette(dib.get()) != NULL)
    {
        QVector<QRgb> pal = getPalette(dib.get());
        if (!isNonePalette(pal))
            result.setColorTable(pal);
    }

    *image = result;
    return true;
}

QVariant FreeImageHandler::option(ImageOption option) const
{
    Q_UNUSED(option)
    return QVariant();
}

void FreeImageHandler::setOption(ImageOption option, const QVariant &value)
{
    Q_UNUSED(option);
    Q_UNUSED(value);
}

bool FreeImageHandler::supportsOption(ImageOption option) const
{
    Q_UNUSED(option)
    return false;
}

FreeImageIO &FreeImageHandler::fiio()
{
    static FreeImageIO io = {ReadProc, WriteProc, SeekProc, TellProc};
    return io;
}

FREE_IMAGE_FORMAT
FreeImageHandler::GetFIF(QIODevice *device, const QByteArray &fmt)
{
    FREE_IMAGE_FORMAT fif =
            FreeImage_GetFileTypeFromHandle(&fiio(), (fi_handle)device);
    device->seek(0);

    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(fmt);
    }

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
    if (raws.indexOf(QString(fmt).toUpper()) != -1)
        return FIF_UNKNOWN;

    if (fifs.indexOf(fif) != -1) {
        return FIF_UNKNOWN;
    }

    return fif;
}

QImage &FreeImageHandler::noneQImage()
{
    static QImage none(0,0,QImage::Format_Invalid);
    return none;
}

bool FreeImageHandler::isNoneQImage(const QImage &qi)
{
    return qi == noneQImage();
}

QVector<QRgb> &FreeImageHandler::nonePalette()
{
    static QVector<QRgb> none = QVector<QRgb>();
    return none;
}

bool FreeImageHandler::isNonePalette(const QVector<QRgb> &pal)
{
    return pal == nonePalette();
}

QImage FreeImageHandler::FIBitmapToQImage(FIBITMAP *dib)
{
    if (!dib || FreeImage_GetImageType(dib) != FIT_BITMAP)
        return noneQImage();
    int width  = FreeImage_GetWidth(dib);
    int height = FreeImage_GetHeight(dib);

    switch (FreeImage_GetBPP(dib))
    {
    case 1:
    {
        QImage result(width, height, QImage::Format_Mono);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 1, 0, 0, 0, true
                    );
        return result;
    }
    case 4: /* NOTE: QImage do not support 4-bit, convert it to 8-bit  */
    {
        QImage result(width, height, QImage::Format_Indexed8);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true
                    );
        return result;
    }
    case 8:
    {
        QImage result(width, height, QImage::Format_Indexed8);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true
                    );
        return result;
    }
    case 16:
        if ( // 5-5-5
             (FreeImage_GetRedMask(dib)   == FI16_555_RED_MASK) &&
             (FreeImage_GetGreenMask(dib) == FI16_555_GREEN_MASK) &&
             (FreeImage_GetBlueMask(dib)  == FI16_555_BLUE_MASK))
        {
            QImage result(width, height, QImage::Format_RGB555);
            FreeImage_ConvertToRawBits(
                        result.scanLine(0), dib, result.bytesPerLine(), 16,
                        FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                        true
                        );
            return result;
        }
        else // 5-6-5
        {
            QImage result(width, height, QImage::Format_RGB16);
            FreeImage_ConvertToRawBits(
                        result.scanLine(0), dib, result.bytesPerLine(), 16,
                        FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                        true
                        );
            return result;
        }
    case 24:
    {
        QImage result(width, height, QImage::Format_RGB32);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 32,
                    FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                    true
                    );
        return result;
    }
    case 32:
    {
        QImage result(width, height, QImage::Format_ARGB32);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 32,
                    FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                    true
                    );
        return result;
    }
    default:
        break;
    }
    return noneQImage();
}

QVector<QRgb> FreeImageHandler::getPalette(FIBITMAP *dib)
{
    if (dib != NULL &&  FreeImage_GetBPP(dib) <= 8)
    {
        RGBQUAD *pal = FreeImage_GetPalette(dib);
        int nColors   = FreeImage_GetColorsUsed(dib);
        QVector<QRgb> result(nColors);
        for (int i = 0; i < nColors; ++i) // first pass
        {
            QColor c(pal[i].rgbRed,pal[i].rgbBlue,pal[i].rgbBlue, 0xFF);
            result[i] = c.rgba();
        }
        if (FreeImage_IsTransparent(dib)) // second pass
        {
            BYTE *transpTable = FreeImage_GetTransparencyTable(dib);
            int nTransp = FreeImage_GetTransparencyCount(dib);
            for (int i = 0; i  < nTransp; ++i)
            {
                QRgb c = result[i];
                result[i] = qRgba(qRed(c),qGreen(c),qBlue(c),transpTable[i]);
            }
        }
        return result;
    }
    return nonePalette();
}
