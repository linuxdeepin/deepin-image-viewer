/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Deng jinhui<dengjinhui@uniontech.com>
*
* Maintainer: Deng jinhui <dengjinhui@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "unionimage.h"
#include <FreeImage.h>
#include "giflib/cmanagerattributeservice.h"

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QDate>
#include <QTime>
#include <QtMath>
#include <QMovie>
#include <QMatrix>
#include <QPainter>
#include <QSvgGenerator>
#include <QImageReader>
#include <QtSvg/QSvgRenderer>


#define SAVE_QUAITY_VALUE 100

const QString DATETIME_FORMAT_NORMAL = "yyyy.MM.dd";
const QString DATETIME_FORMAT_EXIF = "yyyy:MM:dd HH:mm";

namespace UnionImage_NameSpace {

//    enum SupportFormat {
//        UNKNOWN = -1,
//        BMP     =  0,
//        ICO     =  1,
//        JPG     =  2,
//        JPE     =  2,
//        JPS     =  2,
//        JPEG    =  2,
//        JNG     =  3,
//        KOALA   =  4,
//        KOA     =  KOALA,
//        LBM     =  5,
//        IFF     =  LBM,
//        MNG     =  6,
//        PBM     =  7,
//        PBMRAW  =  8,
//        PCD     =  9,
//        PCX     =  10,
//        PGM     =  11,
//        PGMRAW  =  12,
//        PNG     =  13,
//        PPM     =  14,
//        PPMRAW  =  15,
//        RAS     =  16,
//        TGA     =  17,
//        TARGA   =  17,
//        TIFF    =  18,
//        WBMP    =  19,
//        PSD     =  20,
//        CUT     =  21,
//        XBM     =  22,
//        XPM     =  23,
//        DDS     =  24,
//        GIF     =  25,
//        FHDR    =  26,
//        FAXG3   =  27,
//        SGI     =  28,
//        EXR     =  29,
//        J2K     =  30,
//        J2C     =  30,
//        JPC     =  30,
//        JP2     =  31,
//        PFM     =  32,
//        PCT     =  33,
//        PIC     =  33,
//        PICT    =  33,
//        RAW     =  34,
//        WEBP    =  35,
//        JXR     =  36
//    };


class UnionImage_Private
{



public:
    UnionImage_Private()
    {
        //FreeImage_Initialise(true);
        //m_freeiamge_formats["UNKNOWN"] = -1;
        m_freeiamge_formats["BMP"]     =  FIF_BMP;
        m_freeiamge_formats["ICO"]     =  FIF_ICO;
        m_freeiamge_formats["JPG"]     =  FIF_JPEG;
        m_freeiamge_formats["JPE"]     =  FIF_JPEG;
        m_freeiamge_formats["JPS"]     =  FIF_JPEG;
        m_freeiamge_formats["JPEG"]    =  FIF_JPEG;
        m_freeiamge_formats["JNG"]     =  FIF_JNG;
        m_freeiamge_formats["KOALA"]   =  FIF_KOALA;
        m_freeiamge_formats["KOA"]     =  FIF_KOALA;
        m_freeiamge_formats["LBM"]     =  FIF_LBM;
        m_freeiamge_formats["IFF"]     =  FIF_LBM;
        m_freeiamge_formats["MNG"]     =  FIF_MNG;
        m_freeiamge_formats["PBM"]     =  FIF_PBM;
        m_freeiamge_formats["PBMRAW"]  =  FIF_PBMRAW;
        m_freeiamge_formats["PCD"]     =  FIF_PCD;
        m_freeiamge_formats["PCX"]     =  FIF_PCX;
        m_freeiamge_formats["PGM"]     =  FIF_PGM;
        m_freeiamge_formats["PGMRAW"]  =  FIF_PGMRAW;
        m_freeiamge_formats["PNG"]     =  FIF_PNG;
        m_freeiamge_formats["PPM"]     =  FIF_PPM;
        m_freeiamge_formats["PPMRAW"]  =  FIF_PPMRAW;
        m_freeiamge_formats["RAS"]     =  FIF_RAS;
        m_freeiamge_formats["TGA"]     =  FIF_TARGA;
        m_freeiamge_formats["TARGA"]   =  FIF_TARGA;
        m_freeiamge_formats["TIFF"]    =  FIF_TIFF;//use qt
        m_freeiamge_formats["TIF"]     =  FIF_TIFF;//use qt
        m_freeiamge_formats["WBMP"]    =  FIF_WBMP;
        m_freeiamge_formats["PSD"]     =  FIF_PSD;
        m_freeiamge_formats["CUT"]     =  FIF_CUT;
        m_freeiamge_formats["XBM"]     =  FIF_XBM;
        m_freeiamge_formats["XPM"]     =  FIF_XPM;
        m_freeiamge_formats["DDS"]     =  FIF_DDS;
        m_freeiamge_formats["GIF"]     =  FIF_GIF;
        //m_freeiamge_formats["HDR"]     =  FIF_HDR;//FHDR covert failed
        m_freeiamge_formats["FAX"]     =  FIF_FAXG3;
        m_freeiamge_formats["G3"]      =  FIF_FAXG3;//FAXG3
        m_freeiamge_formats["SGI"]     =  FIF_SGI;
        m_freeiamge_formats["EXR"]     =  FIF_EXR;
        m_freeiamge_formats["J2K"]     =  FIF_J2K;
        m_freeiamge_formats["J2C"]     =  FIF_J2K;
        m_freeiamge_formats["JPC"]     =  FIF_J2K;
        m_freeiamge_formats["JP2"]     =  FIF_JP2;
        //m_freeiamge_formats["PFM"]     =  FIF_PFM;covert failed
        m_freeiamge_formats["PCT"]     =  FIF_PICT;
        m_freeiamge_formats["PIC"]     =  FIF_PICT;
        m_freeiamge_formats["PICT"]    =  FIF_PICT;
        m_freeiamge_formats["PIC"]     =  FIF_PICT;
        m_freeiamge_formats["RAW"]     =  FIF_RAW;
        m_freeiamge_formats["WEBP"]    =  FIF_WEBP;
        m_freeiamge_formats["JXR"]     =  FIF_JXR;
        m_movie_formats["MNG"]         =  FIF_MNG;
        m_movie_formats["GIF"]         =  FIF_GIF;
        m_qtSupported << "BMP" << "JPG" << "JPEG" << "PNG" << "PBM"
                      << "PGM" << "PPM" << "PNM" << "WBMP" << "WEBP"
                      << "SVG" << "ICNS" << "GIF" << "MNG" << "TIF"
                      << "TIFF" << "BMP" << "XPM" << "MRW" << "DNG"
                      << "RAF"  << "CR2" << "MEF" << "RAW" << "ORF"
                      << "NEF" ;

        m_not_support_rotate_format << "PFM" << "HDR" << "GIF" << "MNG";
        m_canSave << "BMP" << "JPG" << "JPEG" << "PNG" << "PBM"
                  << "PGM" << "PPM" << "PNM" << "WBMP" << "WEBP"
                  << "SVG" << "TGA" << "XPM" << "ICO" << "ICNS"
                  << "PSD" << "G3" << "J2C" << "J2K" << "JNG"
                  << "JP2" << "PCD" << "PCX" << "PCT"
                  << "PICT" << "PIC" << "RAS";
    }
    ~UnionImage_Private()
    {

    }
    QMutex freeimage_mutex;
    QStringList m_not_support_rotate_format;
    QStringList m_qtSupported;
    QHash<QString, int> m_freeiamge_formats;
    QHash<QString, int> m_movie_formats;
    QStringList m_canSave;
};

static UnionImage_Private union_image_private;

/**
 * @brief noneQImage
 * @return QImage
 * 返回空图片
 */
UNIONIMAGESHARED_EXPORT QImage noneQImage()
{
    static QImage none(0, 0, QImage::Format_Invalid);
    return none;
}

UNIONIMAGESHARED_EXPORT const QStringList unionImageSupportFormat()
{
    static QStringList res;
    if (res.empty()) {
        QStringList list = union_image_private.m_freeiamge_formats.keys();
        for (const QString &i : union_image_private.m_freeiamge_formats.keys()) {
            if (!list.contains(i))
                list.append(i.toLower());
        }
        for (const QString &i : union_image_private.m_qtSupported) {
            if (!list.contains(i))
                list.append(i);
        }
        for (const QString &i : union_image_private.m_qtSupported) {
            if (!list.contains(i))
                list.append(i.toLower());
        }
        res.append(list);
    }
    return res;
}

UNIONIMAGESHARED_EXPORT const QStringList supportStaticFormat()
{
    return (union_image_private.m_freeiamge_formats.keys() << union_image_private.m_qtSupported);
}

UNIONIMAGESHARED_EXPORT const QStringList supportMovieFormat()
{
    return (union_image_private.m_movie_formats.keys());
}

/**
 * @brief size2Human
 * @param bytes
 * @author LMH
 * @return QString
 * 照片尺寸转化为QString格式
 */
UNIONIMAGESHARED_EXPORT QString size2Human(const qlonglong bytes)
{
    qlonglong kb = 1024;
    if (bytes < kb) {
        return QString::number(bytes) + " B";
    } else if (bytes < kb * kb) {
        QString vs = QString::number(static_cast<double>(bytes) / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            return QString::number(static_cast<int>(vs.toDouble())) + " KB";
        } else {
            return vs + " KB";
        }
    } else if (bytes < kb * kb * kb) {
        QString vs = QString::number(static_cast<double>(bytes) / kb / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            return QString::number(static_cast<int>(vs.toDouble())) + " MB";
        } else {
            return vs + " MB";
        }
    } else {
        return QString::number(bytes);
    }
}

/**
 * @brief getFileFormat
 * @param path
 * @author LMH
 * @return QString
 * 文件路径获取文件后缀名
 */
UNIONIMAGESHARED_EXPORT const QString getFileFormat(const QString &path)
{
    QFileInfo fi(path);
    QString suffix = fi.suffix();
    return suffix;
}

/**
 * @brief string2DateTime
 * @param time
 * @author LMH
 * @return QDateTime
 * string转换时间
 */
UNIONIMAGESHARED_EXPORT QDateTime string2DateTime(const QString &time)
{
    QDateTime dt = QDateTime::fromString(time, DATETIME_FORMAT_EXIF);
    if (! dt.isValid()) {
        dt = QDateTime::fromString(time, DATETIME_FORMAT_NORMAL);
    }
    return dt;
}

/**
 * @brief getMetaData
 * @param model
 * @param dib
 * @author LMH
 * @return QMap<QString, QString>
 * 内部接口获取图片的信息
 */
UNIONIMAGESHARED_EXPORT QMap<QString, QString> getMetaData(FREE_IMAGE_MDMODEL model, FIBITMAP *dib)
{
    QMap<QString, QString> mdMap;  // key-data
    FITAG *tag = nullptr;
    FIMETADATA *mdhandle = nullptr;
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

/**
 * @brief FIBitmapToQImage
 * @param dib
 * @return QImage
 * 由FreeImage转到QImage
 */
UNIONIMAGESHARED_EXPORT QImage FIBitmap2QImage(FIBITMAP *dib)
{
    if (!dib || FreeImage_GetImageType(dib) == FIT_UNKNOWN)
        return noneQImage();
    int width  = static_cast<int>(FreeImage_GetWidth(dib));
    int height = static_cast<int>(FreeImage_GetHeight(dib));
    int depth = static_cast<int>(FreeImage_GetBPP(dib));
    switch (depth) {
    case 1: {
        QImage result(width, height, QImage::Format_Mono);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 1, 0, 0, 0, true
        );
        return result;
    }
    case 4: { /* NOTE: QImage do not support 4-bit, convert it to 8-bit  */
        QImage result(width, height, QImage::Format_Indexed8);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true
        );
        return result;
    }
    case 8: {
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
            (FreeImage_GetBlueMask(dib)  == FI16_555_BLUE_MASK)) {
            QImage result(width, height, QImage::Format_RGB555);
            FreeImage_ConvertToRawBits(
                result.scanLine(0), dib, result.bytesPerLine(), 16,
                FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                true
            );
            return result;
        } else { // 5-6-5
            QImage result(width, height, QImage::Format_RGB16);
            FreeImage_ConvertToRawBits(
                result.scanLine(0), dib, result.bytesPerLine(), 16,
                FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                true
            );
            return result;
        }
    case 24: {
        QImage result(width, height, QImage::Format_RGB32);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
            true
        );
        return result;
    }
    case 32: {
        QImage result(width, height, QImage::Format_ARGB32);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
            true
        );
        return result;
    }
    case 48:
    case 64:
    case 96:
    //128位暂不支持
    case 128:
    default:
        break;
    }
    return noneQImage();
}

/**
 * @brief QImgeToFIBitMap
 * @param img
 * @return FIBITMAP*
 * 由QImage转到FreeImage
 */

UNIONIMAGESHARED_EXPORT FIBITMAP *QImge2FIBitMap(QImage img)
{
    if (img.isNull()) {
        return nullptr;
    }
    int width = img.width();
    int height = img.height();
    uint depth = static_cast<uint>(img.depth());
    QImage::Format format = img.format();
    switch (format) {
    //The image is invalid.
    case QImage::Format_Invalid:
        return nullptr;
    //The image is stored using 1-bit per pixel.(MSB)
    case QImage::Format_Mono:
    //The image is stored using 1-bit per pixel.(LSB)
    case QImage::Format_MonoLSB: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth, 0, 0, 0,
                            true);
        return res;
    }
    /* NOTE: QImage do not support 4-bit*/

    //可能会将彩色图转为灰度图
    //The image is stored using an 8-bit alpha only format.
    case QImage::Format_Alpha8:
    //The image is stored using an 8-bit grayscale format.
    case QImage::Format_Grayscale8:
    //The image is stored using 8-bit indexes into a colormap.
    case QImage::Format_Indexed8: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth, 0, 0, 0,
                            true);
        return res;
    }
    //The image is stored using a 16-bit RGB format (5-6-5)
    case QImage::Format_RGB16: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), 16,
                            FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                            true);
        return res;
    }
    //The image is stored using a 16-bit RGB format (5-5-5). The unused most significant bit is always zero.
    case QImage::Format_RGB555: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), 16,
                            FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                            true);
        return res;
    }

    //32-bit
    case QImage::Format_RGB32: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }
    case QImage::Format_ARGB32: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }
    //24-bit
    case QImage::Format_RGB666:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_RGB888: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }

    //FreeImage is not support
    //The image is stored using a 16-bit RGB format (4-4-4). The unused bits are always zero.
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
    default:
        break;
    }
    return nullptr;
}

/**
 * @brief readFile2FIBITMAP
 * @param path
 * @param flags
 * @author LMH
 * @return FIBITMAP
 * 由QString路径转换成FIBITMAP指针
 */
UNIONIMAGESHARED_EXPORT FIBITMAP *readFile2FIBITMAP(const QString &path, int flags FI_DEFAULT(0))
{
    const QByteArray ba = path.toUtf8();
    const char *pc = ba.data();
    const FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(pc);
    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        FIBITMAP *dib = FreeImage_Load(fif, pc, flags);
        return dib;
    }
    return nullptr;
}

UNIONIMAGESHARED_EXPORT bool canSave(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(path.toUtf8().data());
    if (fif != FIF_UNKNOWN) {
        // Check that the dib can be saved in this format
        if (union_image_private.m_canSave.contains(union_image_private.m_freeiamge_formats.key(fif))) {
            return true;
        }
    }
    QFileInfo info(path);
    if (union_image_private.m_canSave.contains(info.suffix().toUpper()))
        return true;
    return false;
}



/**
 * @brief writeFIBITMAPToFile
 * @param dib
 * @param path
 * @param flag
 * @return
 */
UNIONIMAGESHARED_EXPORT bool writeFIBITMAPToFile(FIBITMAP *dib, const QString &path, int flag = 0)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    BOOL bSuccess = FALSE;
    const QByteArray ba = path.toUtf8();
    const char *pc = ba.data();
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(pc);
    if (fif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(fif, dib, pc, flag);
    }
    return bSuccess;
}

UNIONIMAGESHARED_EXPORT QString unionImageVersion()
{
    QString ver;
//    ver.append("Core version :");
//    ver.append(QString(FreeImage_GetVersion()));
//    ver.append("\n");
    ver.append("UnionImage Version:");
    ver.append("1.0.0");
    ver.append("\n");
    return ver;
}



UNIONIMAGESHARED_EXPORT bool creatNewImage(QImage &res, int width, int height, int depth, SupportType type)
{
    Q_UNUSED(type);
    FIBITMAP *dib = FreeImage_Allocate(width, height, depth);
    res = FIBitmap2QImage(dib);
    FreeImage_Unload(dib);
    return true;
}

QString PrivateDetectImageFormat(const QString &filepath);
UNIONIMAGESHARED_EXPORT bool loadStaticImageFromFile(const QString path, QImage &res, QString &errorMsg, const QString &format_bar)
{
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(temp_path.data());
    if (f != FIF_UNKNOWN && f != union_image_private.m_freeiamge_formats[file_suffix_upper]) {
        file_suffix_upper = union_image_private.m_freeiamge_formats.key(f);
    }
    if (f == FIF_TIFF) {
        file_suffix_upper = "TIFF";
    }
    QString file_suffix_lower = file_suffix_upper.toLower();
    if (union_image_private.m_qtSupported.contains(file_suffix_upper)) {
        QImageReader reader;
        QImage res_qt;
        reader.setFileName(path);
        if (format_bar.isEmpty()) {
            reader.setFormat(file_suffix_lower.toLatin1());
        } else {
            reader.setFormat(format_bar.toLatin1());
        }
        reader.setAutoTransform(true);
        res_qt = reader.read();
        if (res_qt.isNull()) {
            //try old loading method
            QString format = PrivateDetectImageFormat(path);
            QImageReader readerF(path, format.toLatin1());
            QImage try_res;
            readerF.setAutoTransform(true);
            if (readerF.canRead()) {
                try_res = readerF.read();
            } else {
                errorMsg = "can't read image:" + readerF.errorString() + format;
                try_res = QImage(path);
            }
            if (try_res.isNull()) {
                errorMsg = "load image by qt faild, use format:" + reader.format() + " ,path:" + path;
                res = QImage();
                return false;
            }
            errorMsg = "use old method to load QImage";
            res = try_res;
            return true;
        }
        errorMsg = "use QImage";
        res = res_qt;
        return true;
    } else {
        if (f != FREE_IMAGE_FORMAT::FIF_UNKNOWN || union_image_private.m_freeiamge_formats.contains(file_suffix_upper)) {
            if (f == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
                f = FREE_IMAGE_FORMAT(union_image_private.m_freeiamge_formats[file_suffix_upper]);
            FIBITMAP *dib = FreeImage_Load(f, temp_path.data());
            if (nullptr == dib) {
                errorMsg = "image load faild, format:" + union_image_private.m_freeiamge_formats.key(f) + " ,path:" + temp_path;
                //FreeImage_Unload(dib);
                res = QImage();
                return false;
            }
//            uint depth = FreeImage_GetBPP(dib); //just for test
//            Q_UNUSED(depth);
            //32位以上图片qImage不支持,强行读取和转换可能会乱码
            res = QImage(FIBitmap2QImage(dib));
            if (res.isNull()) {
                errorMsg = "convert to QImage faild" + union_image_private.m_freeiamge_formats.key(f) + " ,path:" + temp_path;
                FreeImage_Unload(dib);
                res = QImage();
                return false;
            }
            FreeImage_Unload(dib);
            errorMsg = "";
            return true;
        }
        return false;
    }
}

UNIONIMAGESHARED_EXPORT QString detectImageFormat(const QString &path)
{
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(path.toUtf8().data());
    QString res = union_image_private.m_freeiamge_formats.key(f);
    if (res.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return "";
        }

        //    const QByteArray data = file.read(1024);
        const QByteArray data = file.read(64);

        // Check bmp file.
        if (data.startsWith("BM")) {
            return "BMP";
        }

        // Check dds file.
        if (data.startsWith("DDS")) {
            return "DDS";
        }

        // Check gif file.
        if (data.startsWith("GIF8")) {
            return "GIF";
        }

        // Check Max OS icons file.
        if (data.startsWith("icns")) {
            return "ICNS";
        }

        // Check jpeg file.
        if (data.startsWith("\xff\xd8")) {
            return "JPG";
        }

        // Check mng file.
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return "MNG";
        }

        // Check net pbm file (BitMap).
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return "PBM";
        }

        // Check pgm file (GrayMap).
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return "PGM";
        }

        // Check ppm file (PixMap).
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return "PPM";
        }

        // Check png file.
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return "PNG";
        }

        // Check svg file.
        if (data.indexOf("<svg") > -1) {
            return "SVG";
        }

        // TODO(xushaohua): tga file is not supported yet.

        // Check tiff file.
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return "TIFF";
        }

        // TODO(xushaohua): Support wbmp file.

        // Check webp file.
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return "WEBP";
        }

        // Check xbm file.
        if (data.indexOf("#define max_width ") > -1 &&
                data.indexOf("#define max_height ") > -1) {
            return "XBM";
        }

        // Check xpm file.
        if (data.startsWith("/* XPM */")) {
            return "XPM";
        }
        return "";
    }
    return res;
}

UNIONIMAGESHARED_EXPORT bool isNoneQImage(const QImage &qi)
{
    return (qi == noneQImage());
}

UNIONIMAGESHARED_EXPORT bool rotateImage(int angel, QImage &image)
{
    if (angel % 90 != 0)
        return false;
    if (image.isNull()) {
        return false;
    }
    QImage image_copy(image);
    if (!image_copy.isNull()) {
        QMatrix rotatematrix;
        rotatematrix.rotate(angel);
        image = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
        return true;
    }
    return false;
}

UNIONIMAGESHARED_EXPORT bool rotateImageFIle(int angel, const QString &path, QString &erroMsg)
{
    if (angel % 90 != 0) {
        erroMsg = "unsupported angel";
        return false;
    }
    QString format = detectImageFormat(path);
    if (format == "SVG") {
        QImage image_copy;
        if (!loadStaticImageFromFile(path, image_copy, erroMsg)) {
            erroMsg = "rotate load QImage faild, path:" + path + "  ,format:+" + format;
            return false;
        }
        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setViewBox(QRect(0, 0, image_copy.width(), image_copy.height()));
        QPainter rotatePainter;
        rotatePainter.begin(&generator);
        rotatePainter.resetTransform();
        rotatePainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        int realangel = angel / 90;
        if (realangel > 0) {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(image_copy.width(), 0);
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        } else {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(0, image_copy.height());
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        }
        rotatePainter.drawImage(image_copy.rect(), image_copy.scaled(image_copy.width(), image_copy.height()));
        rotatePainter.resetTransform();
        generator.setSize(QSize(image_copy.width(), image_copy.height()));
        rotatePainter.end();
        return true;
    } else if (format == "JPG" || format == "JPEG") {
        QImage image_copy(path, "JPG");
        if (!image_copy.isNull()) {
            QMatrix rotatematrix;
            rotatematrix.rotate(angel);
            image_copy = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
            image_copy.save(path, "jpg", SAVE_QUAITY_VALUE);
            return true;
        }
    }
    FIBITMAP *dib = readFile2FIBITMAP(path);
    if (nullptr == dib) {
        erroMsg = "unsupported format";
        FreeImage_Unload(dib);
        return false;
    }
    FIBITMAP *rotateRes = FreeImage_Rotate(dib, -angel);
    if (rotateRes) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -angel);

            FreeImage_SetThumbnail(rotateRes, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }
    QByteArray temp_path;
    temp_path.append(path);
    FREE_IMAGE_FORMAT f = FREE_IMAGE_FORMAT(union_image_private.m_freeiamge_formats[QFileInfo(path).suffix().toUpper()]);
    if (f == FREE_IMAGE_FORMAT::FIF_UNKNOWN) {
        erroMsg = "rotate image format error";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    if (!writeFIBITMAPToFile(rotateRes, path)) {
        erroMsg = "rotate image save faild, unkown format";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    FreeImage_Unload(dib);
    FreeImage_Unload(rotateRes);
    erroMsg = "";
    return true;
}

UNIONIMAGESHARED_EXPORT bool rotateImageFIleWithImage(int angel, QImage &img, const QString &path, QString &erroMsg)
{
    if (angel % 90 != 0) {
        erroMsg = "unsupported angel";
        return false;
    }
    QImage image_copy;
    if (img.isNull()) return false;
    else image_copy = img;

    QString format = detectImageFormat(path);
    if (format == "SVG") {
        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setViewBox(QRect(0, 0, image_copy.width(), image_copy.height()));
        QPainter rotatePainter;
        rotatePainter.begin(&generator);
        rotatePainter.resetTransform();
        rotatePainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        int realangel = angel / 90;
        if (realangel > 0) {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(image_copy.width(), 0);
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        } else {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(0, image_copy.height());
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        }
        rotatePainter.drawImage(image_copy.rect(), image_copy.scaled(image_copy.width(), image_copy.height()));
        rotatePainter.resetTransform();
        generator.setSize(QSize(image_copy.width(), image_copy.height()));
        rotatePainter.end();
        return true;
    } else if (format == "JPG" || format == "JPEG") {
        QImage image_copy(path, "JPG");
        if (!image_copy.isNull()) {
            QMatrix rotatematrix;
            rotatematrix.rotate(angel);
            image_copy = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
            image_copy.save(path, "jpg", SAVE_QUAITY_VALUE);
            return true;
        }
    }
    FIBITMAP *dib = readFile2FIBITMAP(path);
    if (nullptr == dib) {
        erroMsg = "unsupported format";
        FreeImage_Unload(dib);
        return false;
    }
    FIBITMAP *rotateRes = FreeImage_Rotate(dib, -angel);
    if (rotateRes) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -angel);
            FreeImage_SetThumbnail(rotateRes, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }
    QByteArray temp_path;
    temp_path.append(path);
    FREE_IMAGE_FORMAT f = FREE_IMAGE_FORMAT(union_image_private.m_freeiamge_formats[QFileInfo(path).suffix().toUpper()]);
    if (f == FREE_IMAGE_FORMAT::FIF_UNKNOWN) {
        erroMsg = "rotate image format error";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    img = FIBitmap2QImage(rotateRes);
    if (!writeFIBITMAPToFile(rotateRes, path)) {
        erroMsg = "rotate image save faild, unkown format";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    FreeImage_Unload(dib);
    FreeImage_Unload(rotateRes);
    erroMsg = "";
    return true;
}

UNIONIMAGESHARED_EXPORT QMap<QString, QString> getAllMetaData(const QString &path)
{
    QMutexLocker mutex(&union_image_private.freeimage_mutex);

    //qDebug() << "threadid:" << QThread::currentThread() << "getAllMetaData locking ....";

    FIBITMAP *dib = readFile2FIBITMAP(path, FIF_LOAD_NOPIXELS);
    QMap<QString, QString> admMap;
    admMap.unite(getMetaData(FIMD_EXIF_MAIN, dib));
    admMap.unite(getMetaData(FIMD_EXIF_EXIF, dib));
    admMap.unite(getMetaData(FIMD_EXIF_GPS, dib));
    admMap.unite(getMetaData(FIMD_EXIF_MAKERNOTE, dib));
    admMap.unite(getMetaData(FIMD_EXIF_INTEROP, dib));
    admMap.unite(getMetaData(FIMD_IPTC, dib));
    //移除秒　　2020/6/5 DJH
    if (admMap.contains("DateTime")) {
        QDateTime time = QDateTime::fromString(admMap["DateTime"], "yyyy:MM:dd hh:mm");
        admMap["DateTime"] = time.toString("yyyy/MM/dd HH:mm");
    }
    // Basic extended data
    QFileInfo info(path);
    if (admMap.isEmpty()) {
        QDateTime emptyTime(QDate(0, 0, 0), QTime(0, 0));
        admMap.insert("DateTimeOriginal",  emptyTime.toString("yyyy/MM/dd HH:mm"));
        admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));
    } else {
        // Exif version 0231
        QString qsdto = admMap.value("DateTimeOriginal");
        QString qsdtd = admMap.value("DateTimeDigitized");
        QDateTime ot = QDateTime::fromString(qsdto, "yyyy/MM/dd HH:mm");
        QDateTime dt = QDateTime::fromString(qsdtd, "yyyy/MM/dd HH:mm");
        if (! ot.isValid()) {
            // Exif version 0221
            QString qsdt = admMap.value("DateTime");
            ot = QDateTime::fromString(qsdt, "yyyy/MM/dd HH:mm");
            dt = ot;

            // NO valid date information
            if (! ot.isValid()) {
//                admMap.insert("DateTimeOriginal", info.created().toString("yyyy/MM/dd HH:mm:dd"));
                admMap.insert("DateTimeOriginal", info.birthTime().toString("yyyy/MM/dd HH:mm"));
                admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));
            }
        }
        admMap.insert("DateTimeOriginal", ot.toString("yyyy/MM/dd HH:mm"));
        admMap.insert("DateTimeDigitized", dt.toString("yyyy/MM/dd HH:mm"));

    }

//    // The value of width and height might incorrect
    QImageReader reader(path);
    int w = reader.size().width();
    w = w > 0 ? w : static_cast<int>(FreeImage_GetWidth(dib));
    int h = reader.size().height();
    h = h > 0 ? h : static_cast<int>(FreeImage_GetHeight(dib));
    admMap.insert("Dimension", QString::number(w) + "x" + QString::number(h));

    admMap.insert("FileName", info.fileName());
    admMap.insert("FileFormat", getFileFormat(path));
    admMap.insert("FileSize", size2Human(info.size()));
    FreeImage_Unload(dib);
    //qDebug() <<  QThread::currentThread() << "getAllMetaData lock end";
    return admMap;
}

UNIONIMAGESHARED_EXPORT bool isImageSupportRotate(const QString &path)
{
    return canSave(path);
}

UNIONIMAGESHARED_EXPORT bool isSupportsReading(const QString &path)
{
    const FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.toUtf8().data());
    return (fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif);
}

UNIONIMAGESHARED_EXPORT bool isSupportsWriting(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.toUtf8().data());
    return (fif != FIF_UNKNOWN) && FreeImage_FIFSupportsWriting(fif);
}

UNIONIMAGESHARED_EXPORT const QString getOrientation(const QString &path)
{
    FIBITMAP *dib = readFile2FIBITMAP(path, FIF_LOAD_NOPIXELS);
    auto datas = getMetaData(FIMD_EXIF_MAIN, dib);
    if (datas.isEmpty()) {
        return QString();
    }
    FreeImage_Unload(dib);
    return datas["Orientation"];
}

bool getThumbnail(QImage &res, const QString &path)
{
    FIBITMAP *dib = readFile2FIBITMAP(path);
    res = FIBitmap2QImage(FreeImage_GetThumbnail(dib));
    FreeImage_Unload(dib);
    return true;
}

class UnionMovieImagePrivate : public QObject
{

public:
    explicit UnionMovieImagePrivate(UnionMovieImage *parent): q_ptr(parent)
    {

    }
    ~UnionMovieImagePrivate()
    {
        CManagerAttributeService::getInstance()->setCouldRun(false);
        CManagerAttributeService::getInstance()->GifFreeFile();
    }


    void setPathAndBegin(const QString &path)
    {
        CManagerAttributeService::getInstance()->setfilePathWithSignalPlay(path);
        QObject::connect(CManagerAttributeService::getInstance(), &CManagerAttributeService::emitImageSignal, this, [ = ](QImage image, bool isFirst) {
            Q_UNUSED(isFirst);
            res = image;
        });
    }

    int getCurrent()
    {
        return currentIndex;
    }

    void reset()
    {
        CManagerAttributeService::getInstance()->setCouldRun(false);
        CManagerAttributeService::getInstance()->GifFreeFile();

        delete r;
        r = nullptr;
        errMsg = "";
        res = QImage();
        currentFormat = FIF_UNKNOWN;
        currentIndex = 0;
        frames = 0;
    }

    void setIndex(int i)
    {
        currentIndex = i;
    }
public:
    UnionMovieImage *const q_ptr;
    Q_DECLARE_PUBLIC(UnionMovieImage)
    QImageReader *r = nullptr;
    QString errMsg = "";
    QImage res;
    FREE_IMAGE_FORMAT currentFormat = FIF_UNKNOWN;
    int currentIndex = 0;
    int frames = 0;
};

UnionMovieImage::UnionMovieImage(): d_ptr(new UnionMovieImagePrivate(this))
{

}

UnionMovieImage::~UnionMovieImage()
{
    Q_D(UnionMovieImage);
    delete d;
}

void UnionMovieImage::setFileName(const QString &path)
{
    Q_D(UnionMovieImage);
    d->reset();
    QString errMsg;
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(temp_path.data());
    if (!union_image_private.m_movie_formats.contains(file_suffix_upper) || !union_image_private.m_movie_formats.values().contains(f)) {
        errMsg = "static Image";
    } else {
        switch (f) {
        case FIF_GIF: {
            d->setPathAndBegin(path);
            d->currentFormat = FIF_GIF;
        }
        break;
        case FIF_MNG: {
            d->r = new QImageReader;
            d->r->setFileName(path);
            if (d->r->canRead()) {
                d->currentFormat = FIF_MNG;
                d->frames = d->r->imageCount();
            }
        }
        break;
        default:
            break;
        }
    }
}

QImage UnionMovieImage::next()
{
    Q_D(UnionMovieImage);
    switch (d->currentFormat) {
    case FIF_GIF: {
        return d->res;
    }
    case FIF_MNG: {
        int temp = d->currentIndex;
        d->setIndex(temp + 1);
        if (temp + 1 >= d->frames) {
            d->setIndex(0);
        }
        d->res = d->r->read();
        d->r->jumpToNextImage();
        break;
    }
    default:
        break;
    }
    return d->res;
}

QString PrivateDetectImageFormat(const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    const QByteArray data = file.read(1024);

    // Check bmp file.
    if (data.startsWith("BM")) {
        return "bmp";
    }

    // Check dds file.
    if (data.startsWith("DDS")) {
        return "dds";
    }

    // Check gif file.
    if (data.startsWith("GIF8")) {
        return "gif";
    }

    // Check Max OS icons file.
    if (data.startsWith("icns")) {
        return "icns";
    }

    // Check jpeg file.
    if (data.startsWith("\xff\xd8")) {
        return "jpg";
    }

    // Check mng file.
    if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
        return "mng";
    }

    // Check net pbm file (BitMap).
    if (data.startsWith("P1") || data.startsWith("P4")) {
        return "pbm";
    }

    // Check pgm file (GrayMap).
    if (data.startsWith("P2") || data.startsWith("P5")) {
        return "pgm";
    }

    // Check ppm file (PixMap).
    if (data.startsWith("P3") || data.startsWith("P6")) {
        return "ppm";
    }

    // Check png file.
    if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
        return "png";
    }

    // Check svg file.
    if (data.indexOf("<svg") > -1) {
        return "svg";
    }

    // TODO(xushaohua): tga file is not supported yet.

    // Check tiff file.
    if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
        // big-endian, little-endian.
        return "tiff";
    }

    // TODO(xushaohua): Support wbmp file.

    // Check webp file.
    if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
        return "webp";
    }

    // Check xbm file.
    if (data.indexOf("#define max_width ") > -1 &&
            data.indexOf("#define max_height ") > -1) {
        return "xbm";
    }

    // Check xpm file.
    if (data.startsWith("/* XPM */")) {
        return "xpm";
    }
    return "";
}

};



