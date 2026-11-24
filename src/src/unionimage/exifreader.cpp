// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exifreader.h"

#include <QByteArrayView>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QXmlStreamReader>
#include <QtEndian>
#include <QImageReader>

#include <libexif/exif-data.h>
#include <libheif/heif.h>
#include <libraw.h>
#include <tiffio.h>
#include <zlib.h>

#include <cmath>
#include <iterator>
#include <memory>
#include <optional>

namespace LibUnionImage_NameSpace {

static QByteArray readJpegExifPayload(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.read(2) != QByteArrayLiteral("\xff\xd8"))
        return {};

    const qint64 fileSize = file.size();
    char marker = 0;
    while (file.getChar(&marker) && static_cast<uchar>(marker) == 0xff) {
        // JPEG allows extra 0xff bytes before a marker code.
        do {
            if (!file.getChar(&marker))
                return {};
        } while (static_cast<uchar>(marker) == 0xff);

        const auto code = static_cast<uchar>(marker);
        if (code == 0x00 || code == 0xd8 || code == 0xd9 || code == 0xda) // Stop before scan data.
            break;
        if (code == 0x01 || (code >= 0xd0 && code <= 0xd7)) // TEM and restart markers have no length.
            continue;

        char lengthBytes[2];
        if (file.read(lengthBytes, 2) != 2)
            break;

        // The length field includes its own two bytes.
        const int payloadSize = qFromBigEndian<quint16>(lengthBytes) - 2;
        if (payloadSize < 0 || payloadSize > fileSize - file.pos())
            break;

        if (code == 0xe1) { // APP1 can contain EXIF or other metadata, such as XMP.
            const QByteArray payload = file.read(payloadSize);
            if (payload.size() != payloadSize)
                break;
            if (payload.startsWith(QByteArrayLiteral("Exif\0\0")))
                return payload;
        } else if (!file.seek(file.pos() + payloadSize)) {
            break;
        }
    }

    return {};
}

// Keep metadata reads bounded even when a container allows much larger chunks.
static constexpr qint64 maxExifChunkSize = 1024 * 1024;

static QByteArray readPngExifPayload(QFile &file, qint64 imageEnd)
{
    if (imageEnd - file.pos() < 8 || file.read(8) != QByteArrayLiteral("\x89PNG\r\n\x1a\n"))
        return {};

    constexpr qint64 crcSize = 4;
    while (imageEnd - file.pos() >= 8) {
        const QByteArray header = file.read(8);
        if (header.size() != 8)
            break;

        const auto chunkSize = qFromBigEndian<quint32>(header.constData());
        const QByteArrayView type(header.constData() + 4, 4);
        if (type == "IEND" || chunkSize > imageEnd - file.pos() - crcSize)
            break;
        if (type == "eXIf") {
            if (chunkSize > maxExifChunkSize)
                break;
            const QByteArray payload = file.read(chunkSize);
            const QByteArray checksum = file.read(crcSize);
            if (payload.size() != chunkSize || checksum.size() != crcSize)
                return {};
            auto crc = crc32(0, reinterpret_cast<const Bytef *>(type.data()), 4);
            crc = crc32(crc, reinterpret_cast<const Bytef *>(payload.constData()), chunkSize);
            return crc == qFromBigEndian<quint32>(checksum.constData()) ? payload : QByteArray();
        }
        if (!file.seek(file.pos() + chunkSize + crcSize))
            break;
    }
    return {};
}

static QByteArray readPngExifPayload(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? readPngExifPayload(file, file.size()) : QByteArray();
}

static QByteArray readWebpExifPayload(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QByteArray header = file.read(12);
    if (header.size() != 12 || !header.startsWith("RIFF") || !header.endsWith("WEBP"))
        return {};

    const qint64 riffEnd = 8 + qint64(qFromLittleEndian<quint32>(header.constData() + 4));
    if (riffEnd < 12 || riffEnd > file.size())
        return {};

    while (riffEnd - file.pos() >= 8) {
        const QByteArray chunk = file.read(8);
        if (chunk.size() != 8)
            break;

        const auto chunkSize = qFromLittleEndian<quint32>(chunk.constData() + 4);
        // RIFF pads odd-sized chunks to an even boundary.
        const qint64 paddedSize = qint64(chunkSize) + (chunkSize % 2);
        if (paddedSize > riffEnd - file.pos())
            break;
        if (chunk.startsWith("EXIF")) {
            if (chunkSize > maxExifChunkSize)
                break;
            const QByteArray payload = file.read(chunkSize);
            return payload.size() == chunkSize ? payload : QByteArray();
        }
        if (!file.seek(file.pos() + paddedSize))
            break;
    }
    return {};
}

static QByteArray readIcoExifPayload(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QByteArray header = file.read(6);
    if (header.size() != 6 || !header.startsWith(QByteArrayLiteral("\0\0\x01\0")))
        return {};

    constexpr qint64 entrySize = 16;
    const qint64 tableSize = qFromLittleEndian<quint16>(header.constData() + 4) * entrySize;
    const qint64 tableEnd = header.size() + tableSize;
    const qint64 fileSize = file.size();
    if (tableEnd > fileSize)
        return {};
    const QByteArray entries = file.read(tableSize);
    if (entries.size() != tableSize)
        return {};

    // An ICO may contain several PNG or DIB images. Keep each PNG read inside its icon entry.
    for (qint64 pos = 0; pos < tableSize; pos += entrySize) {
        const char *entry = entries.constData() + pos;
        const qint64 size = qFromLittleEndian<quint32>(entry + 8);
        const qint64 offset = qFromLittleEndian<quint32>(entry + 12);
        if (offset < tableEnd || offset > fileSize || size > fileSize - offset || !file.seek(offset))
            continue;
        QByteArray payload = readPngExifPayload(file, offset + size);
        if (!payload.isEmpty())
            return payload;
    }
    return {};
}

struct PhotoshopMetadata {
    QByteArray exif;
    QByteArray xmp;
};

static PhotoshopMetadata readPhotoshopMetadata(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QByteArray header = file.read(26);
    if (header.size() != 26 || !header.startsWith("8BPS"))
        return {};
    const quint16 version = qFromBigEndian<quint16>(header.constData() + 4);
    if (version != 1 && version != 2)
        return {};

    const qint64 fileSize = file.size();
    const QByteArray colorLength = file.read(4);
    if (colorLength.size() != 4)
        return {};
    const qint64 colorDataSize = qFromBigEndian<quint32>(colorLength.constData());
    if (colorDataSize > fileSize - file.pos() || !file.seek(file.pos() + colorDataSize))
        return {};

    const QByteArray resourcesLength = file.read(4);
    if (resourcesLength.size() != 4)
        return {};
    const qint64 resourcesSize = qFromBigEndian<quint32>(resourcesLength.constData());
    if (resourcesSize > fileSize - file.pos())
        return {};
    const qint64 resourcesEnd = file.pos() + resourcesSize;

    PhotoshopMetadata metadata;
    while (resourcesEnd - file.pos() >= 7) {
        const QByteArray resource = file.read(7);
        if (resource.size() != 7 || !resource.startsWith("8BIM"))
            break;
        const quint16 resourceId = qFromBigEndian<quint16>(resource.constData() + 4);
        const int nameSize = static_cast<uchar>(resource[6]);
        // The Pascal string includes its length byte and is padded to an even size.
        const int nameRemaining = nameSize + ((nameSize + 1) % 2);
        if (nameRemaining + 4 > resourcesEnd - file.pos() || !file.seek(file.pos() + nameRemaining))
            break;

        const QByteArray dataLength = file.read(4);
        if (dataLength.size() != 4)
            break;
        const qint64 dataSize = qFromBigEndian<quint32>(dataLength.constData());
        const qint64 paddedSize = dataSize + (dataSize % 2);
        if (paddedSize > resourcesEnd - file.pos())
            break;
        const qint64 nextResource = file.pos() + paddedSize;

        if (dataSize <= maxExifChunkSize) {
            if (resourceId == 1058 && metadata.exif.isEmpty())
                metadata.exif = file.read(dataSize);
            else if (resourceId == 1060 && metadata.xmp.isEmpty())
                metadata.xmp = file.read(dataSize);
        }
        if ((!metadata.exif.isEmpty() && !metadata.xmp.isEmpty()) || !file.seek(nextResource))
            break;
    }
    return metadata;
}

static bool skipGifColorTable(QFile &file, uchar flags)
{
    if (!(flags & 0x80))
        return true;
    const qint64 tableSize = 3 * (2 << (flags & 7));
    return tableSize <= file.size() - file.pos() && file.seek(file.pos() + tableSize);
}

static bool readGifSubBlocks(QFile &file, QByteArray *packet = nullptr)
{
    const qint64 fileSize = file.size();
    char byte;
    while (file.getChar(&byte)) {
        const auto length = static_cast<uchar>(byte);
        if (length == 0)
            return true;
        if (length > fileSize - file.pos())
            return false;
        if (!packet) {
            if (!file.seek(file.pos() + length))
                return false;
            continue;
        }
        // XMP's UTF-8 packet includes the bytes GIF readers interpret as block lengths.
        if (packet->size() + length + 1 > maxExifChunkSize)
            return false;
        const QByteArray block = file.read(length);
        if (block.size() != length)
            return false;
        packet->append(byte);
        packet->append(block);
    }
    return false;
}

static QByteArray readGifXmpPacket(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QByteArray header = file.read(13);
    if (header.size() != 13 || (!header.startsWith("GIF89a") && !header.startsWith("GIF87a"))
        || !skipGifColorTable(file, static_cast<uchar>(header[10]))) {
        return {};
    }

    char marker;
    while (file.getChar(&marker)) {
        if (marker == '\x2c') {
            const QByteArray descriptor = file.read(9);
            char minimumCodeSize;
            if (descriptor.size() != 9 || !skipGifColorTable(file, static_cast<uchar>(descriptor[8]))
                || !file.getChar(&minimumCodeSize) || !readGifSubBlocks(file)) {
                break;
            }
        } else if (marker == '\x21') {
            char label;
            if (!file.getChar(&label))
                break;
            if (static_cast<uchar>(label) == 0xff) {
                char length;
                if (!file.getChar(&length) || static_cast<uchar>(length) != 11)
                    break;
                const QByteArray application = file.read(11);
                if (application.size() != 11)
                    break;
                if (application == "XMP DataXMP") {
                    QByteArray packet;
                    if (!readGifSubBlocks(file, &packet))
                        return {};
                    QByteArray trailer(1, '\x01');
                    for (int value = 255; value > 0; --value)
                        trailer.append(static_cast<char>(value));
                    // The sub-block terminator may consume the last zero byte of the magic trailer.
                    const qsizetype end = packet.lastIndexOf(trailer);
                    if (end < 0 || (packet.size() != end + trailer.size()
                                    && packet.mid(end + trailer.size()) != QByteArray(1, '\0'))) {
                        return {};
                    }
                    return packet.first(end);
                }
            }
            if (!readGifSubBlocks(file))
                break;
        } else {
            break;
        }
    }
    return {};
}

static QByteArray readHeifExifPayload(const QString &path)
{
    const std::unique_ptr<heif_context, decltype(&heif_context_free)> context(heif_context_alloc(), &heif_context_free);
    const QByteArray nativePath = QFile::encodeName(path);
    if (!context || heif_context_read_from_file(context.get(), nativePath.constData(), nullptr).code != heif_error_Ok)
        return {};

    heif_image_handle *primaryImage = nullptr;
    if (heif_context_get_primary_image_handle(context.get(), &primaryImage).code != heif_error_Ok)
        return {};
    const std::unique_ptr<heif_image_handle, decltype(&heif_image_handle_release)> image(primaryImage, &heif_image_handle_release);

    heif_item_id exifId;
    if (heif_image_handle_get_list_of_metadata_block_IDs(image.get(), "Exif", &exifId, 1) != 1)
        return {};
    constexpr size_t offsetSize = sizeof(quint32);
    constexpr size_t tiffHeaderSize = 8;
    const size_t itemSize = heif_image_handle_get_metadata_size(image.get(), exifId);
    if (itemSize < offsetSize + tiffHeaderSize || itemSize > maxExifChunkSize)
        return {};

    QByteArray item(itemSize, Qt::Uninitialized);
    if (heif_image_handle_get_metadata(image.get(), exifId, item.data()).code != heif_error_Ok)
        return {};

    // The offset is relative to the first byte after the four-byte offset field.
    const quint32 tiffOffset = qFromBigEndian<quint32>(item.constData());
    if (tiffOffset > itemSize - offsetSize - tiffHeaderSize)
        return {};
    return item.sliced(offsetSize + tiffOffset);
}

template <typename T>
static T readTiffValue(const char *data, ExifByteOrder order)
{
    return order == EXIF_BYTE_ORDER_INTEL ? qFromLittleEndian<T>(data) : qFromBigEndian<T>(data);
}

static constexpr qsizetype tiffEntrySize = 12;

static QByteArrayView findTiffTag(QByteArrayView data, quint32 ifdOffset, quint16 tag,
                                  ExifByteOrder order)
{
    if (data.size() < 2 || ifdOffset > data.size() - 2)
        return {};

    const char *entry = data.data() + ifdOffset;
    const quint16 count = readTiffValue<quint16>(entry, order);
    entry += 2;
    if (count > (data.size() - ifdOffset - 2) / tiffEntrySize)
        return {};

    for (quint16 i = 0; i < count; ++i, entry += tiffEntrySize) {
        if (readTiffValue<quint16>(entry, order) == tag)
            return QByteArrayView(entry, tiffEntrySize);
    }
    return {};
}

static std::optional<double> readFlashCompensation(QByteArrayView note, QByteArrayView tiff,
                                                   ExifByteOrder order, const QString &make)
{
    if (note.isEmpty())
        return {};

    if (note.startsWith(QByteArrayView("Nikon\0\x02", 7))) {
        // Nikon type 3. The embedded TIFF header starts ten bytes into the note.
        if (note.size() < 18)
            return {};
        const QByteArrayView nikonTiff = note.sliced(10);
        const QByteArrayView byteOrder = nikonTiff.first(2);
        if (byteOrder != QByteArrayView("II") && byteOrder != QByteArrayView("MM"))
            return {};
        const auto order = byteOrder == QByteArrayView("II") ? EXIF_BYTE_ORDER_INTEL : EXIF_BYTE_ORDER_MOTOROLA;
        if (readTiffValue<quint16>(nikonTiff.data() + 2, order) != 42)
            return {};
        const auto ifdOffset = readTiffValue<quint32>(nikonTiff.data() + 4, order);
        const auto entry = findTiffTag(nikonTiff, ifdOffset, 0x0012, order);
        if (entry.isEmpty() || readTiffValue<quint16>(entry.data() + 2, order) != EXIF_FORMAT_UNDEFINED
            || readTiffValue<quint32>(entry.data() + 4, order) != 4) {
            return {};
        }
        // Nikon's inline payload is three signed bytes (value, multiplier, divisor) and a pad.
        const double divisor = static_cast<qint8>(entry[10]);
        if (divisor == 0)
            return {};
        return static_cast<qint8>(entry[8]) * static_cast<qint8>(entry[9]) / divisor;
    }

    if (!make.startsWith(QStringLiteral("Canon"), Qt::CaseInsensitive))
        return {};

    // Locate the MakerNote in the original TIFF so its value offsets can be checked.
    const auto ifd0Offset = readTiffValue<quint32>(tiff.data() + 4, order);
    const auto exifPointer = findTiffTag(tiff, ifd0Offset, EXIF_TAG_EXIF_IFD_POINTER, order);
    if (exifPointer.isEmpty() || readTiffValue<quint16>(exifPointer.data() + 2, order) != EXIF_FORMAT_LONG
        || readTiffValue<quint32>(exifPointer.data() + 4, order) != 1) {
        return {};
    }
    const auto exifOffset = readTiffValue<quint32>(exifPointer.data() + 8, order);
    const auto noteTag = findTiffTag(tiff, exifOffset, EXIF_TAG_MAKER_NOTE, order);
    if (noteTag.isEmpty())
        return {};
    const auto noteOffset = readTiffValue<quint32>(noteTag.data() + 8, order);
    if (noteOffset > tiff.size() || note.size() > tiff.size() - noteOffset
        || tiff.sliced(noteOffset, note.size()) != note) {
        return {};
    }

    // Canon ShotInfo (0x0004) stores flash compensation at index 15. Its offset is TIFF-relative.
    const auto entry = findTiffTag(note, 0, 0x0004, order);
    if (entry.isEmpty())
        return {};
    const auto format = readTiffValue<quint16>(entry.data() + 2, order);
    const auto count = readTiffValue<quint32>(entry.data() + 4, order);
    const auto offset = readTiffValue<quint32>(entry.data() + 8, order);
    constexpr qsizetype flashIndex = 15;
    constexpr qsizetype shortSize = sizeof(qint16);
    if ((format != EXIF_FORMAT_SHORT && format != EXIF_FORMAT_SSHORT) || count <= flashIndex
        || offset < noteOffset || offset > tiff.size()) {
        return {};
    }

    // The value block must follow the MakerNote directory, including its next-IFD pointer.
    const qsizetype valueOffset = offset - noteOffset;
    const qsizetype directorySize = sizeof(quint16) + readTiffValue<quint16>(note.data(), order) * tiffEntrySize
                                    + sizeof(quint32);
    if (valueOffset < directorySize || valueOffset > note.size() || count > (note.size() - valueOffset) / shortSize)
        return {};

    const char *shotInfo = note.data() + valueOffset;
    // The first short in ShotInfo records the size of the array in bytes.
    if (readTiffValue<quint16>(shotInfo, order) != count * shortSize)
        return {};

    const int encoded = readTiffValue<qint16>(shotInfo + flashIndex * shortSize, order);
    const int magnitude = qAbs(encoded);
    const int fraction = magnitude % 32;
    // Canon reserves 0x0c and 0x14 for exact thirds of a stop.
    double value = magnitude / 32;
    if (fraction == 0x0c)
        value += 1.0 / 3;
    else if (fraction == 0x14)
        value += 2.0 / 3;
    else
        value += fraction / 32.0;
    return encoded < 0 ? -value : value;
}

static QString formatExifCaptureTime(const QString &stamp)
{
    if (stamp.size() != 19 || stamp.at(10) != QLatin1Char(' '))
        return {};

    // Keep the recorded capture time across local DST transitions.
    const QDate date = QDate::fromString(stamp.left(10), QStringLiteral("yyyy:MM:dd"));
    const QTime time = QTime::fromString(stamp.mid(11), QStringLiteral("HH:mm:ss"));
    if (!date.isValid() || !time.isValid())
        return {};
    return date.toString(QStringLiteral("yyyy/MM/dd")) + QLatin1Char(' ') + time.toString(QStringLiteral("HH:mm"));
}

template <typename SensitivityReader>
static uint readSensitivity(uint legacy, uint type, const SensitivityReader &readValue)
{
    if (legacy > 0 && legacy < 65535)
        return legacy;

    constexpr uint sos = 1;
    constexpr uint rei = 2;
    constexpr uint iso = 4;
    // SensitivityType encodes which extended fields represent the camera's sensitivity.
    constexpr uint masks[] = {sos | rei | iso, sos, rei, iso, sos | rei, sos | iso, rei | iso, sos | rei | iso};
    const uint mask = type < std::size(masks) ? masks[type] : masks[0];
    constexpr struct {
        uint flag;
        ExifTag tag;
    } fields[] = {
        {iso, EXIF_TAG_ISO_SPEED},
        {rei, EXIF_TAG_RECOMMENDED_EXPOSURE_INDEX},
        {sos, EXIF_TAG_STANDARD_OUTPUT_SENSITIVITY},
    };
    for (const auto &field : fields) {
        if (mask & field.flag) {
            const uint value = readValue(field.tag);
            if (value > 0)
                return value;
        }
    }
    return 0;
}

template <typename TagReader>
static void readExifFields(const TagReader &readTag, QMap<QString, QString> &metadata)
{
    struct Field {
        ExifIfd ifd;
        ExifTag tag;
        const char *key;
    };
    // Keep the keys expected by InformationDialog.qml. Later entries take priority.
    static constexpr Field fields[] = {
        {EXIF_IFD_0, EXIF_TAG_MODEL, "Model"},
        {EXIF_IFD_EXIF, EXIF_TAG_LENS_MODEL, "LensType"},
        {EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE, "ApertureValue"},
        {EXIF_IFD_EXIF, EXIF_TAG_FNUMBER, "ApertureValue"},
        {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM, "ExposureProgram"},
        {EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH, "FocalLength"},
        {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE, "ExposureMode"},
        {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME, "ExposureTime"},
        {EXIF_IFD_EXIF, EXIF_TAG_FLASH, "Flash"},
        {EXIF_IFD_EXIF, EXIF_TAG_MAX_APERTURE_VALUE, "MaxApertureValue"},
        {EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE, "ColorSpace"},
        {EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE, "MeteringMode"},
        {EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE, "WhiteBalance"},
    };
    for (const auto &field : fields) {
        const QString value = readTag(field.ifd, field.tag);
        if (!value.isEmpty())
            metadata.insert(QString::fromLatin1(field.key), value);
    }

    const auto readSensitivityTag = [&](ExifTag tag) { return readTag(EXIF_IFD_EXIF, tag).toUInt(); };
    const uint iso = readSensitivity(readSensitivityTag(EXIF_TAG_ISO_SPEED_RATINGS),
                                     readSensitivityTag(EXIF_TAG_SENSITIVITY_TYPE), readSensitivityTag);
    if (iso > 0)
        metadata.insert(QStringLiteral("ISOSpeedRatings"), QString::number(iso));

    const QString captureTime = formatExifCaptureTime(readTag(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL));
    if (!captureTime.isEmpty())
        metadata.insert(QStringLiteral("DateTimeOriginal"), captureTime);
}

static QString formatExifEntry(ExifEntry &entry, ExifByteOrder order)
{
    if (entry.tag == EXIF_TAG_ISO_SPEED_RATINGS || entry.tag == EXIF_TAG_SENSITIVITY_TYPE) {
        if (entry.format != EXIF_FORMAT_SHORT || !entry.data || entry.size < sizeof(quint16))
            return {};
        return QString::number(exif_get_short(entry.data, order));
    }
    if (entry.tag == EXIF_TAG_ISO_SPEED || entry.tag == EXIF_TAG_RECOMMENDED_EXPOSURE_INDEX
        || entry.tag == EXIF_TAG_STANDARD_OUTPUT_SENSITIVITY) {
        if (entry.format != EXIF_FORMAT_LONG || !entry.data || entry.size < sizeof(quint32))
            return {};
        return QString::number(exif_get_long(entry.data, order));
    }

    char value[1024] = {};
    exif_entry_get_value(&entry, value, sizeof(value));
    return QString::fromLocal8Bit(value).trimmed();
}

static void addFlashCompensation(QByteArrayView note, QByteArrayView tiff, ExifByteOrder order,
                                 const QString &make, QMap<QString, QString> &metadata)
{
    const auto compensation = readFlashCompensation(note, tiff, order, make);
    if (!compensation)
        return;
    QString value = QString::number(*compensation, 'f', 2);
    if (*compensation > 0)
        value.prepend(QLatin1Char('+'));
    metadata.insert(QStringLiteral("FlashExposureComp"), value + QStringLiteral(" EV"));
}

struct TiffTagValue {
    QByteArrayView bytes;
    ExifFormat format;
    quint32 count;
};

static std::optional<TiffTagValue> readTiffTagValue(QByteArrayView tiff, quint32 directory,
                                                    ExifTag tag, ExifByteOrder order)
{
    if (directory < 8)
        return {};
    const auto entry = findTiffTag(tiff, directory, tag, order);
    if (entry.isEmpty())
        return {};
    const auto type = readTiffValue<quint16>(entry.data() + 2, order);
    if (type < EXIF_FORMAT_BYTE || type > EXIF_FORMAT_DOUBLE)
        return {};
    const auto format = static_cast<ExifFormat>(type);
    const auto count = readTiffValue<quint32>(entry.data() + 4, order);
    const quint64 size = quint64(count) * exif_format_get_size(format);
    if (size == 0 || size > static_cast<quint64>(tiff.size()))
        return {};
    if (size <= 4)
        return TiffTagValue{entry.sliced(8, size), format, count};
    const auto offset = readTiffValue<quint32>(entry.data() + 8, order);
    if (offset < 8 || offset > tiff.size() - static_cast<qsizetype>(size))
        return {};
    return TiffTagValue{tiff.sliced(offset, size), format, count};
}

static void readLargeExif(QByteArrayView tiff, ExifData *exif, QMap<QString, QString> &metadata)
{
    ExifByteOrder order;
    if (tiff.startsWith(QByteArrayView("II\x2a\0", 4)))
        order = EXIF_BYTE_ORDER_INTEL;
    else if (tiff.startsWith(QByteArrayView("MM\0\x2a", 4)))
        order = EXIF_BYTE_ORDER_MOTOROLA;
    else
        return;
    exif_data_set_byte_order(exif, order);

    const quint32 ifd0 = readTiffValue<quint32>(tiff.data() + 4, order);
    const auto pointer = readTiffTagValue(tiff, ifd0, EXIF_TAG_EXIF_IFD_POINTER, order);
    quint32 exifIfd = 0;
    if (pointer && pointer->format == EXIF_FORMAT_LONG && pointer->count == 1)
        exifIfd = readTiffValue<quint32>(pointer->bytes.data(), order);

    const auto readTag = [&](ExifIfd ifd, ExifTag tag) -> QString {
        const auto field = readTiffTagValue(tiff, ifd == EXIF_IFD_0 ? ifd0 : exifIfd, tag, order);
        if (!field)
            return {};
        if (field->format == EXIF_FORMAT_ASCII) {
            auto text = field->bytes.first(qMin(field->bytes.size(), qsizetype(1024)));
            const auto terminator = text.indexOf('\0');
            if (terminator >= 0)
                text = text.first(terminator);
            return QString::fromLocal8Bit(text.data(), text.size()).trimmed();
        }
        if (field->format != EXIF_FORMAT_SHORT && field->format != EXIF_FORMAT_LONG
            && field->format != EXIF_FORMAT_RATIONAL) {
            return {};
        }
        // Format one bounded numeric value using the same labels as the libexif path.
        const auto valueSize = exif_format_get_size(field->format);
        auto bytes = field->bytes.first(valueSize).toByteArray();
        ExifEntry entry{};
        entry.parent = exif->ifd[ifd];
        entry.tag = tag;
        entry.format = field->format;
        entry.components = 1;
        entry.data = reinterpret_cast<unsigned char *>(bytes.data());
        entry.size = valueSize;
        return formatExifEntry(entry, order);
    };
    readExifFields(readTag, metadata);
    const auto note = readTiffTagValue(tiff, exifIfd, EXIF_TAG_MAKER_NOTE, order);
    if (note && note->format == EXIF_FORMAT_UNDEFINED)
        addFlashCompensation(note->bytes, tiff, order, readTag(EXIF_IFD_0, EXIF_TAG_MAKE), metadata);
}

static void readExif(QByteArray payload, QMap<QString, QString> &metadata)
{
    constexpr char signature[] = "Exif\0\0";
    constexpr qsizetype signatureSize = sizeof(signature) - 1;
    const bool prefixed = payload.startsWith(QByteArrayView(signature, signatureSize));
    const auto tiff = QByteArrayView(payload).sliced(prefixed ? signatureSize : 0);
    if (tiff.size() < 8)
        return;
    if (!tiff.startsWith(QByteArrayView("II\x2a\0", 4)) && !tiff.startsWith(QByteArrayView("MM\0\x2a", 4)))
        return;

    const std::unique_ptr<ExifData, decltype(&exif_data_unref)> exif(exif_data_new(), &exif_data_unref);
    if (!exif)
        return;

    // libexif caps input to an APP1-sized buffer. Larger containers may place IFDs beyond that limit.
    if (tiff.size() + signatureSize > 0xfffe) {
        readLargeExif(tiff, exif.get(), metadata);
        return;
    }

    // Only show tags stored in the file. The default option can add missing mandatory tags.
    exif_data_unset_option(exif.get(), EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    if (!prefixed)
        payload.prepend(signature, signatureSize);
    exif_data_load_data(exif.get(), reinterpret_cast<const unsigned char *>(payload.constData()),
                        static_cast<unsigned int>(payload.size()));
    const auto order = exif_data_get_byte_order(exif.get());
    const auto readTag = [&](ExifIfd ifd, ExifTag tag) -> QString {
        ExifEntry *entry = exif_content_get_entry(exif->ifd[ifd], tag);
        return entry ? formatExifEntry(*entry, order) : QString();
    };
    readExifFields(readTag, metadata);

    const ExifEntry *note = exif_content_get_entry(exif->ifd[EXIF_IFD_EXIF], EXIF_TAG_MAKER_NOTE);
    if (note && note->data) {
        addFlashCompensation(QByteArrayView(reinterpret_cast<const char *>(note->data), note->size),
                             QByteArrayView(payload).sliced(signatureSize), order,
                             readTag(EXIF_IFD_0, EXIF_TAG_MAKE), metadata);
    }
}

static bool isRawFormat(const QByteArray &format)
{
    static constexpr QByteArrayView formats[] = {
        "raw", "arw", "cr2", "cr3", "crw", "dcr", "dng", "erf", "kdc", "mef", "mrw",
        "nef", "nrw", "orf", "pef", "raf", "rw2", "rwl", "sr2", "srf", "srw", "x3f",
    };
    for (const auto candidate : formats) {
        if (candidate == format)
            return true;
    }
    return false;
}

static void readRawCaptureStamp(void *context, int tag, int type, int count, unsigned int,
                                void *stream, INT64)
{
    if (tag != EXIF_TAG_DATE_TIME_ORIGINAL || type != EXIF_FORMAT_ASCII || count != 20)
        return;

    // LibRaw restores the stream position after calling the EXIF parser callback.
    char stamp[19];
    auto *input = static_cast<LibRaw_abstract_datastream *>(stream);
    if (input->read(stamp, 1, sizeof(stamp)) == sizeof(stamp))
        *static_cast<QByteArray *>(context) = QByteArray(stamp, sizeof(stamp));
}

static QString formatExposureTime(double seconds)
{
    if (seconds < 1) {
        const double reciprocal = 1.0 / seconds;
        const double denominator = std::round(reciprocal);
        if (denominator > 1 && std::abs(reciprocal - denominator) <= reciprocal * 0.005)
            return QStringLiteral("1/%1 sec.").arg(denominator, 0, 'f', 0);
    }
    return QStringLiteral("%1 sec.").arg(seconds, 0, 'g', 6);
}

static constexpr QLatin1StringView rdfNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#");
static constexpr QLatin1StringView svgNamespace("http://www.w3.org/2000/svg");

static bool isXmpCameraNamespace(QStringView uri)
{
    return uri == QLatin1StringView("http://ns.adobe.com/exif/1.0/")
        || uri == QLatin1StringView("http://cipa.jp/exif/1.0/")
        || uri == QLatin1StringView("http://ns.adobe.com/tiff/1.0/");
}

static bool isXmpCameraProperty(QStringView name)
{
    constexpr const char *supported[] = {
        "Model", "LensModel", "PhotographicSensitivity", "ISOSpeedRatings", "SensitivityType",
        "ISOSpeed", "RecommendedExposureIndex", "StandardOutputSensitivity", "FNumber",
        "FocalLength", "ExposureTime", "DateTimeOriginal",
    };
    for (const auto property : supported) {
        if (name == QLatin1StringView(property))
            return true;
    }
    return false;
}

static QString readXmpProperty(QXmlStreamReader &xml)
{
    QString value;
    int depth = 1;
    int listItemDepth = 0;
    bool hasListItem = false;
    bool oversized = false;
    while (!xml.atEnd() && depth > 0) {
        xml.readNext();
        if (xml.isStartElement()) {
            ++depth;
            if (xml.namespaceUri() == rdfNamespace && xml.name() == QLatin1StringView("li") && !hasListItem) {
                value.clear();
                oversized = false;
                hasListItem = true;
                listItemDepth = depth;
            }
        } else if (xml.isEndElement()) {
            if (depth == listItemDepth)
                listItemDepth = 0;
            --depth;
        } else if (xml.isCharacters() && ((depth == 1 && !hasListItem) || depth == listItemDepth)) {
            if (xml.text().size() > 1024 - value.size())
                oversized = true;
            if (!oversized)
                value += xml.text();
        }
    }
    return oversized ? QString() : value.trimmed();
}

static QMap<QString, QString> readXmpCameraProperties(QXmlStreamReader &xml, bool svg)
{
    QMap<QString, QString> properties;
    QMap<QString, QString> blockProperties;
    int depth = 0;
    int descriptionDepth = 0;
    int svgMetadataDepth = 0;
    qint64 svgMetadataStart = 0;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isDTD() && !xml.entityDeclarations().isEmpty())
            return {};
        if (xml.isStartElement()) {
            ++depth;
            const bool svgElement = xml.namespaceUri() == svgNamespace || xml.namespaceUri().isEmpty();
            if (svg && depth == 1 && (!svgElement || xml.name() != QLatin1StringView("svg")))
                return {};
            if (svg && !svgMetadataDepth && svgElement && xml.name() == QLatin1StringView("metadata")) {
                svgMetadataDepth = depth;
                svgMetadataStart = xml.characterOffset();
            }
            if (svg && !svgMetadataDepth)
                continue;

            auto &target = svg ? blockProperties : properties;
            if (xml.namespaceUri() == rdfNamespace && xml.name() == QLatin1StringView("Description")) {
                descriptionDepth = depth;
                for (const auto &attribute : xml.attributes()) {
                    const auto value = attribute.value().trimmed();
                    if (isXmpCameraNamespace(attribute.namespaceUri()) && isXmpCameraProperty(attribute.name())
                        && !value.isEmpty() && value.size() <= 1024) {
                        target.insert(attribute.name().toString(), value.toString());
                    }
                }
            } else if (descriptionDepth && depth == descriptionDepth + 1 && isXmpCameraNamespace(xml.namespaceUri())
                       && isXmpCameraProperty(xml.name())) {
                const QString name = xml.name().toString();
                const QString value = readXmpProperty(xml);
                if (!value.isEmpty())
                    target.insert(name, value);
                --depth; // The property reader consumed its closing element.
            }
        } else if (xml.isEndElement()) {
            if (depth == descriptionDepth)
                descriptionDepth = 0;
            if (depth == svgMetadataDepth) {
                if (xml.characterOffset() - svgMetadataStart <= maxExifChunkSize) {
                    for (auto it = blockProperties.cbegin(); it != blockProperties.cend(); ++it) {
                        if (!properties.contains(it.key()))
                            properties.insert(it.key(), it.value());
                    }
                }
                blockProperties.clear();
                svgMetadataDepth = 0;
            }
            --depth;
        }
    }
    return xml.hasError() ? QMap<QString, QString>() : properties;
}

static std::optional<double> readXmpNumber(const QString &value)
{
    const qsizetype slash = value.indexOf(QLatin1Char('/'));
    bool valid;
    double number = (slash < 0 ? value : value.left(slash)).toDouble(&valid);
    if (!valid)
        return {};
    if (slash >= 0) {
        const double denominator = value.mid(slash + 1).toDouble(&valid);
        if (!valid || !std::isfinite(denominator) || denominator <= 0)
            return {};
        number /= denominator;
    }
    return std::isfinite(number) ? std::optional<double>(number) : std::nullopt;
}

static void addXmpCameraProperties(const QMap<QString, QString> &properties, QMap<QString, QString> &metadata)
{
    const auto copyField = [&](const char *property, const char *key) {
        const QString value = properties.value(QLatin1StringView(property));
        if (!value.isEmpty())
            metadata.insert(QLatin1StringView(key), value);
    };
    copyField("Model", "Model");
    copyField("LensModel", "LensType");

    uint legacy = properties.value(QStringLiteral("PhotographicSensitivity")).toUInt();
    if (legacy == 0)
        legacy = properties.value(QStringLiteral("ISOSpeedRatings")).toUInt();
    const auto readSensitivityProperty = [&](ExifTag tag) -> uint {
        switch (tag) {
        case EXIF_TAG_ISO_SPEED:
            return properties.value(QStringLiteral("ISOSpeed")).toUInt();
        case EXIF_TAG_RECOMMENDED_EXPOSURE_INDEX:
            return properties.value(QStringLiteral("RecommendedExposureIndex")).toUInt();
        case EXIF_TAG_STANDARD_OUTPUT_SENSITIVITY:
            return properties.value(QStringLiteral("StandardOutputSensitivity")).toUInt();
        default:
            return 0;
        }
    };
    const uint iso = readSensitivity(legacy, properties.value(QStringLiteral("SensitivityType")).toUInt(),
                                     readSensitivityProperty);
    if (iso > 0)
        metadata.insert(QStringLiteral("ISOSpeedRatings"), QString::number(iso));
    const auto fNumber = readXmpNumber(properties.value(QStringLiteral("FNumber")));
    if (fNumber && *fNumber > 0)
        metadata.insert(QStringLiteral("ApertureValue"), QStringLiteral("f/%1").arg(*fNumber));
    const auto focalLength = readXmpNumber(properties.value(QStringLiteral("FocalLength")));
    if (focalLength && *focalLength > 0)
        metadata.insert(QStringLiteral("FocalLength"), QStringLiteral("%1 mm").arg(*focalLength));
    const auto exposure = readXmpNumber(properties.value(QStringLiteral("ExposureTime")));
    if (exposure && *exposure > 0 && std::isfinite(1.0 / *exposure))
        metadata.insert(QStringLiteral("ExposureTime"), formatExposureTime(*exposure));

    QString timestamp = properties.value(QStringLiteral("DateTimeOriginal"));
    if (timestamp.size() < 16 || timestamp[10] != QLatin1Char('T'))
        return;

    // Use an explicit offset for parsing so local DST does not change a zone-less capture time.
    if (!timestamp.endsWith(QLatin1Char('Z')) && timestamp.indexOf(QLatin1Char('+'), 16) < 0
        && timestamp.indexOf(QLatin1Char('-'), 16) < 0) {
        timestamp.append(QLatin1Char('Z'));
    }
    const QDateTime captureTime = QDateTime::fromString(timestamp, Qt::ISODate);
    if (captureTime.isValid())
        metadata.insert(QStringLiteral("DateTimeOriginal"), captureTime.toString(QStringLiteral("yyyy/MM/dd HH:mm")));
}

static QString formatTiffShort(ExifContent *content, ExifTag tag, quint16 value)
{
    unsigned char buffer[sizeof(value)];
    exif_set_short(buffer, exif_data_get_byte_order(content->parent), value);
    ExifEntry entry{};
    entry.tag = tag;
    entry.format = EXIF_FORMAT_SHORT;
    entry.components = 1;
    entry.data = buffer;
    entry.size = sizeof(buffer);
    entry.parent = content;
    char text[256] = {};
    exif_entry_get_value(&entry, text, sizeof(text));
    return QString::fromLocal8Bit(text).trimmed();
}

static QString readTiffExifTag(TIFF *tiff, ExifContent *content, ExifTag tag)
{
    switch (tag) {
    case EXIF_TAG_LENS_MODEL:
    case EXIF_TAG_DATE_TIME_ORIGINAL: {
        char *value = nullptr;
        return TIFFGetField(tiff, tag, &value) && value ? QString::fromLocal8Bit(value).trimmed() : QString();
    }
    case EXIF_TAG_ISO_SPEED_RATINGS: {
        quint16 count = 0;
        quint16 *values = nullptr;
        return TIFFGetField(tiff, tag, &count, &values) && count && values
            ? QString::number(values[0]) : QString();
    }
    case EXIF_TAG_ISO_SPEED:
    case EXIF_TAG_RECOMMENDED_EXPOSURE_INDEX:
    case EXIF_TAG_STANDARD_OUTPUT_SENSITIVITY: {
        quint32 value = 0;
        return TIFFGetField(tiff, tag, &value) ? QString::number(value) : QString();
    }
    case EXIF_TAG_FNUMBER:
    case EXIF_TAG_APERTURE_VALUE:
    case EXIF_TAG_MAX_APERTURE_VALUE:
    case EXIF_TAG_EXPOSURE_TIME:
    case EXIF_TAG_FOCAL_LENGTH: {
        float value = 0;
        if (!TIFFGetField(tiff, tag, &value) || !std::isfinite(value))
            return {};
        if (tag == EXIF_TAG_APERTURE_VALUE || tag == EXIF_TAG_MAX_APERTURE_VALUE) {
            const auto fNumber = std::exp2(value / 2.0);
            return std::isfinite(fNumber) ? QStringLiteral("f/%1").arg(fNumber, 0, 'f', 1) : QString();
        }
        if (value <= 0)
            return {};
        if (tag == EXIF_TAG_EXPOSURE_TIME)
            return formatExposureTime(value);
        if (tag == EXIF_TAG_FOCAL_LENGTH)
            return QStringLiteral("%1 mm").arg(value);
        return QStringLiteral("f/%1").arg(value);
    }
    case EXIF_TAG_SENSITIVITY_TYPE: {
        quint16 value = 0;
        return TIFFGetField(tiff, tag, &value) ? QString::number(value) : QString();
    }
    case EXIF_TAG_EXPOSURE_PROGRAM:
    case EXIF_TAG_EXPOSURE_MODE:
    case EXIF_TAG_FLASH:
    case EXIF_TAG_COLOR_SPACE:
    case EXIF_TAG_METERING_MODE:
    case EXIF_TAG_WHITE_BALANCE: {
        quint16 value = 0;
        return TIFFGetField(tiff, tag, &value) ? formatTiffShort(content, tag, value) : QString();
    }
    default:
        return {};
    }
}

static void readTiffMetadata(const QString &path, QMap<QString, QString> &metadata)
{
    const std::unique_ptr<TIFFOpenOptions, decltype(&TIFFOpenOptionsFree)> options(TIFFOpenOptionsAlloc(), &TIFFOpenOptionsFree);
    if (!options)
        return;
    TIFFOpenOptionsSetMaxSingleMemAlloc(options.get(), 16 * maxExifChunkSize);
    const QByteArray nativePath = QFile::encodeName(path);
    const std::unique_ptr<TIFF, decltype(&TIFFClose)> tiff(TIFFOpenExt(nativePath.constData(), "r", options.get()), &TIFFClose);
    if (!tiff)
        return;

    char *model = nullptr;
    QString cameraModel;
    if (TIFFGetField(tiff.get(), TIFFTAG_MODEL, &model) && model)
        cameraModel = QString::fromLocal8Bit(model).trimmed();

    uint64_t exifOffset = 0;
    if (!TIFFGetField(tiff.get(), TIFFTAG_EXIFIFD, &exifOffset) || !exifOffset
        || !TIFFReadEXIFDirectory(tiff.get(), exifOffset)) {
        if (!cameraModel.isEmpty())
            metadata.insert(QStringLiteral("Model"), cameraModel);
        return;
    }

    // Reuse libexif's value labels for the enumerated tags from the TIFF directory.
    const std::unique_ptr<ExifData, decltype(&exif_data_unref)> exif(exif_data_new(), &exif_data_unref);
    if (!exif)
        return;
    const auto readTag = [&](ExifIfd ifd, ExifTag tag) -> QString {
        if (ifd == EXIF_IFD_0)
            return tag == EXIF_TAG_MODEL ? cameraModel : QString();
        return readTiffExifTag(tiff.get(), exif->ifd[ifd], tag);
    };
    readExifFields(readTag, metadata);
}

static void readRawMetadata(const QString &path, QMap<QString, QString> &metadata)
{
    QByteArray captureStamp;
    const auto raw = std::make_unique<LibRaw>();
    raw->set_exifparser_handler(readRawCaptureStamp, &captureStamp);

    // open_file populates metadata and leaves pixel unpacking to the image decoder.
    const QByteArray nativePath = QFile::encodeName(path);
    if (raw->open_file(nativePath.constData()) != LIBRAW_SUCCESS || !raw->imgdata.idata.raw_count)
        return;

    const auto &image = raw->imgdata;
    const QString model = QString::fromLocal8Bit(image.idata.model).trimmed();
    if (!model.isEmpty())
        metadata.insert(QStringLiteral("Model"), model);
    const QString lens = QString::fromLocal8Bit(image.lens.Lens).trimmed();
    if (!lens.isEmpty())
        metadata.insert(QStringLiteral("LensType"), lens);

    const auto &settings = image.other;
    const auto isRecorded = [](float value) { return std::isfinite(value) && value > 0; };
    if (isRecorded(settings.iso_speed))
        metadata.insert(QStringLiteral("ISOSpeedRatings"), QString::number(settings.iso_speed));
    if (isRecorded(settings.aperture))
        metadata.insert(QStringLiteral("ApertureValue"), QStringLiteral("f/%1").arg(settings.aperture));
    if (isRecorded(settings.focal_len))
        metadata.insert(QStringLiteral("FocalLength"), QStringLiteral("%1 mm").arg(settings.focal_len));
    if (isRecorded(settings.shutter))
        metadata.insert(QStringLiteral("ExposureTime"), formatExposureTime(settings.shutter));

    // LibRaw's aggregate timestamp can come from DateTimeDigitized or TIFF DateTime.
    const QString captureTime = formatExifCaptureTime(QString::fromLatin1(captureStamp));
    if (!captureTime.isEmpty())
        metadata.insert(QStringLiteral("DateTimeOriginal"), captureTime);
}

static void readXmpMetadata(const QByteArray &packet, QMap<QString, QString> &metadata)
{
    if (packet.isEmpty())
        return;
    QXmlStreamReader xml(packet);
    addXmpCameraProperties(readXmpCameraProperties(xml, false), metadata);
}

class SvgMetadataInput : public QIODevice
{
public:
    explicit SvgMetadataInput(const QString &path)
        : m_file(gzopen(QFile::encodeName(path).constData(), "rb"), &gzclose)
    {
        // gzread also passes through uncompressed SVG files.
        if (m_file)
            open(QIODevice::ReadOnly);
    }

    bool isSequential() const override { return true; }

protected:
    qint64 readData(char *data, qint64 size) override
    {
        if (m_remaining <= 0)
            return 0;
        const int result = gzread(m_file.get(), data, static_cast<unsigned int>(qMin(size, m_remaining)));
        if (result > 0)
            m_remaining -= result;
        return result;
    }

    qint64 writeData(const char *, qint64) override { return -1; }

private:
    std::unique_ptr<gzFile_s, decltype(&gzclose)> m_file;
    // Limit decompression work independently of the size of an XMP metadata block.
    qint64 m_remaining = 64 * 1024 * 1024;
};

static void readSvgXmpMetadata(const QString &path, QMap<QString, QString> &metadata)
{
    SvgMetadataInput input(path);
    if (!input.isOpen())
        return;
    QXmlStreamReader xml(&input);
    addXmpCameraProperties(readXmpCameraProperties(xml, true), metadata);
}

void readExifMetadata(const QString &path, const QByteArray &format, QMap<QString, QString> &metadata)
{
    if (format == "jpeg")
        readExif(readJpegExifPayload(path), metadata);
    else if (format == "png")
        readExif(readPngExifPayload(path), metadata);
    else if (format == "webp")
        readExif(readWebpExifPayload(path), metadata);
    else {
        const QByteArray suffix = QFileInfo(path).suffix().toLatin1().toLower();
        const QByteArray detectedFormat = format.isEmpty() ? suffix : format.toLower();
        if (isRawFormat(detectedFormat) || isRawFormat(suffix))
            readRawMetadata(path, metadata);
        else if (detectedFormat == "tiff" || detectedFormat == "tif")
            readTiffMetadata(path, metadata);
        else if (detectedFormat == "ico")
            readExif(readIcoExifPayload(path), metadata);
        else if (detectedFormat == "gif")
            readXmpMetadata(readGifXmpPacket(path), metadata);
        else if (detectedFormat == "svg" || detectedFormat == "svgz")
            readSvgXmpMetadata(path, metadata);
        else if (detectedFormat == "psd" || detectedFormat == "psb") {
            const auto resources = readPhotoshopMetadata(path);
            readXmpMetadata(resources.xmp, metadata);
            readExif(resources.exif, metadata);
        } else if (detectedFormat == "heif" || detectedFormat == "heic" || detectedFormat == "avif"
                 || detectedFormat == "heifs" || detectedFormat == "heics" || detectedFormat == "avifs")
            readExif(readHeifExifPayload(path), metadata);
    }
}

static int readOrientationFromExif(QByteArray payload)
{
    constexpr char signature[] = "Exif\0\0";
    constexpr qsizetype signatureSize = sizeof(signature) - 1;
    const bool prefixed = payload.startsWith(QByteArrayView(signature, signatureSize));
    const auto tiff = QByteArrayView(payload).sliced(prefixed ? signatureSize : 0);
    if (tiff.size() < 8)
        return 1;
    if (!tiff.startsWith(QByteArrayView("II\x2a\0", 4)) && !tiff.startsWith(QByteArrayView("MM\0\x2a", 4)))
        return 1;

    const std::unique_ptr<ExifData, decltype(&exif_data_unref)> exif(exif_data_new(), &exif_data_unref);
    if (!exif)
        return 1;

    // libexif caps input to an APP1-sized buffer. Larger containers are rare for orientation.
    if (tiff.size() + signatureSize > 0xfffe)
        return 1;

    exif_data_unset_option(exif.get(), EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    if (!prefixed)
        payload.prepend(signature, signatureSize);
    exif_data_load_data(exif.get(), reinterpret_cast<const unsigned char *>(payload.constData()),
                        static_cast<unsigned int>(payload.size()));

    ExifEntry *entry = exif_content_get_entry(exif->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
    if (entry && entry->data && entry->size >= 2) {
        const auto order = exif_data_get_byte_order(exif.get());
        const int orientation = exif_get_short(entry->data, order);
        if (orientation >= 1 && orientation <= 8)
            return orientation;
    }
    return 1;
}

int getExifOrientation(const QString &path)
{
    QImageReader reader(path);
    const QByteArray format = reader.format();

    QByteArray payload;
    if (format == "jpeg")
        payload = readJpegExifPayload(path);
    else if (format == "png")
        payload = readPngExifPayload(path);
    else if (format == "webp")
        payload = readWebpExifPayload(path);
    else {
        const QByteArray suffix = QFileInfo(path).suffix().toLatin1().toLower();
        const QByteArray detectedFormat = format.isEmpty() ? suffix : format.toLower();
        if (detectedFormat == "ico")
            payload = readIcoExifPayload(path);
        else if (detectedFormat == "heif" || detectedFormat == "heic" || detectedFormat == "avif"
                 || detectedFormat == "heifs" || detectedFormat == "heics" || detectedFormat == "avifs")
            payload = readHeifExifPayload(path);
    }

    return payload.isEmpty() ? 1 : readOrientationFromExif(payload);
}

} // namespace LibUnionImage_NameSpace
