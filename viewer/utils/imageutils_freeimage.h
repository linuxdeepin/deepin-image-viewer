#include "baseutils.h"
#include <FreeImage.h>
#include <QFileInfo>
#include <QImage>
#include <QMap>
#include <QString>

namespace utils {

namespace image {

namespace freeimage {

const QString getFileFormat(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(path.toUtf8().data(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(path.toUtf8().data());
    }

    if (fif == FIF_UNKNOWN) {
        return QString("UNKNOW");
    }
    else {
        return QString(FreeImage_GetFIFMimeType(fif));
    }
}

FIBITMAP * readFileToFIBITMAP(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(path.toUtf8().data(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(path.toUtf8().data());
    }

    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        FIBITMAP *dib = FreeImage_Load(fif, path.toUtf8().data(), 0);
        return dib;
    }

    return NULL;
}


QMap<QString, QString> getMetaData(FREE_IMAGE_MDMODEL model, FIBITMAP *dib)
{
    QMap<QString, QString> mdMap;  // key-data
    FITAG *tag = NULL;
    FIMETADATA *mdhandle = NULL;
    mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);
    if (mdhandle) {
        do {
            mdMap.insert(FreeImage_GetTagKey(tag),
                         FreeImage_TagToString(model, tag));
        } while (FreeImage_FindNextMetadata(mdhandle, &tag));

        FreeImage_FindCloseMetadata(mdhandle);
    }

    return mdMap;
}

QMap<QString, QString> getAllMetaData(const QString &path)
{
    FIBITMAP *dib = readFileToFIBITMAP(path);
    QMap<QString, QString> admMap;
    admMap.unite(getMetaData(FIMD_EXIF_MAIN, dib));
    admMap.unite(getMetaData(FIMD_EXIF_EXIF, dib));
    admMap.unite(getMetaData(FIMD_EXIF_GPS, dib));
    admMap.unite(getMetaData(FIMD_EXIF_MAKERNOTE, dib));
    admMap.unite(getMetaData(FIMD_EXIF_INTEROP, dib));
    admMap.unite(getMetaData(FIMD_IPTC, dib));

    // Basic extended data
    QFileInfo info(path);
    if (admMap.isEmpty()) {
        using namespace utils::base;
        admMap.insert("DateTimeOriginal", timeToString(info.created()));
        admMap.insert("DateTimeDigitized", timeToString(info.lastModified()));
    }
    admMap.insert("Resolution", QString::number(FreeImage_GetWidth(dib)) + "x"
                   + QString::number(FreeImage_GetHeight(dib)));
    admMap.insert("FileName", info.fileName());
    admMap.insert("FileFormat", getFileFormat(path));
    admMap.insert("FileSize", utils::base::sizeToHuman(info.size()));

    FreeImage_Unload(dib);

    return admMap;
}

FIBITMAP * makeThumbnail(const QString &path, int size) {
    const char *szPathName = path.toUtf8().data();
    FIBITMAP *dib = NULL;
    int flags = 0;              // default load flag
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(szPathName);
    if(fif == FIF_UNKNOWN) {
        if (FIF_UNKNOWN == (fif = FreeImage_GetFIFFromFilename(szPathName))) {
            return NULL;
        }
    }

    // for JPEG images, we can speedup the loading part
    // using LibJPEG downsampling feature while loading the image...
    if(fif == FIF_JPEG) {
        flags |= size << 16;
        // load the dib
        dib = FreeImage_Load(fif, szPathName, flags);
        if(!dib) return NULL;
    } else {
        // any cases other than the JPEG case: load the dib ...
        if(fif == FIF_RAW) {
            // ... except for RAW images, try to load the embedded JPEG preview
            // or default to RGB 24-bit ...
            flags = RAW_PREVIEW;
        }
        dib = FreeImage_Load(fif, szPathName, flags);
        if(!dib) return NULL;
    }

    // create the requested thumbnail
    FIBITMAP *thumbnail = FreeImage_MakeThumbnail(dib, size, TRUE);
    FreeImage_Unload(dib);
    return thumbnail;
}

bool isSupportsReading(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(path.toUtf8().data(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(path.toUtf8().data());
    }

    return (fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif);
}

bool isSupportsWriting(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(path.toUtf8().data(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(path.toUtf8().data());
    }

    return (fif != FIF_UNKNOWN) && FreeImage_FIFSupportsWriting(fif);
}

bool writeFIBITMAPToFile(FIBITMAP* dib, const QString &path, int flag = 0) {
    const char *lpszPathName = path.toUtf8().data();
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    BOOL bSuccess = FALSE;
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(lpszPathName);
    if(fif != FIF_UNKNOWN ) {
        // Check that the dib can be saved in this format
        BOOL bCanSave;
        FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);
        if(image_type == FIT_BITMAP) {
            // standard bitmap type
            // check that the plugin has sufficient writing
            // and export capabilities ...
            WORD bpp = FreeImage_GetBPP(dib);
            bCanSave = (FreeImage_FIFSupportsWriting(fif) &&
                        FreeImage_FIFSupportsExportBPP(fif, bpp));
        } else {
            // special bitmap type
            // check that the plugin has sufficient export capabilities
            bCanSave = FreeImage_FIFSupportsExportType(fif, image_type);
        }
        if(bCanSave) {
            bSuccess = FreeImage_Save(fif, dib, lpszPathName, flag);
        }
    }

    return (bSuccess == TRUE) ? true : false;
}

QImage noneQImage()
{
    static QImage none(0,0,QImage::Format_Invalid);
    return none;
}

bool isNoneQImage(const QImage &qi)
{
    return qi == noneQImage();
}

QImage FIBitmapToQImage(FIBITMAP *dib)
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

}  // namespace freeimage

}  // namespace image

}  // namespace utils
