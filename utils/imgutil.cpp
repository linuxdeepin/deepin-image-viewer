#include "utils/imgutil.h"
#include <QDebug>
#include <QFileInfo>
#include <QBuffer>
extern "C" {
#include "libjpeg/jpeg-data.h"
}
namespace {

const int THUMBNAIL_MAX_SIZE = 384;

}  // namespace
namespace utils {

QPixmap getThumbnail(const QString &filePath)
{
    QPixmap pixmap;
    ExifData *ed = exif_data_new_from_file(filePath.toUtf8().data());
    if (ed) {
        // Make sure the image had a thumbnail before trying to write it
        if (ed->data && ed->size) {
            pixmap.loadFromData(ed->data, ed->size);
            // Free the EXIF data
            exif_data_unref(ed);
        } else {
            qDebug() << QString("NO Exif thumbnail in file %1!").arg(filePath);
            pixmap = scaleImage(filePath);
        }
    }
    else {
        pixmap = scaleImage(filePath);
    }

    return pixmap;
}

QPixmap scaleImage(const QString &filePath)
{
    QImage img(filePath);
    QSize targetSize;
    if (img.width() > img.height()) {
        targetSize = QSize(THUMBNAIL_MAX_SIZE,
                           (double)THUMBNAIL_MAX_SIZE / img.width() *
                           img.height());
    }
    else {
        targetSize = QSize((double)THUMBNAIL_MAX_SIZE / img.height() *
                           img.width(), THUMBNAIL_MAX_SIZE);
    }

    return QPixmap::fromImage(img.scaled(targetSize * 2)
                              .scaled(targetSize, Qt::IgnoreAspectRatio,
                                      Qt::SmoothTransformation));
}

QDateTime getCreateDateTime(const QString &filePath)
{
    ExifData *ed = exif_data_new_from_file(filePath.toUtf8().data());
    if (ed) {
        QDateTime dt;
        dt = QDateTime::fromString(readExifTag(ed,
                                               EXIF_IFD_0, EXIF_TAG_DATE_TIME),
                                   "yyyy:MM:dd HH:mm:ss");
        //Free the EXIF data
        exif_data_unref(ed);
        return dt;
    }
    else {
        QFileInfo info(filePath);
        return info.created();
    }
}

QString readExifTag(ExifData *ed, ExifIfd eid, ExifTag tag)
{
    ExifEntry *entry = exif_content_get_entry(ed->ifd[eid], tag);

    if (entry){
        char buf[1024];
        exif_entry_get_value(entry, buf, sizeof(buf));

        if (*buf) {
            return QString(buf).trimmed();
        }
    }

    return QString();
}

/* raw EXIF header data */
const unsigned char kExifHeader[] = {
  0xff, 0xd8, 0xff, 0xe1
};

void saveImageWithExif(const QImage &image, const QString &path, const QString &sourcePath, const QTransform &mat)
{
    if (sourcePath.isEmpty() || !path.endsWith(".jpg", Qt::CaseInsensitive) || path.endsWith(".jpeg", Qt::CaseInsensitive)) {
        image.save(path);
        return;
    }
    ExifData *ed = exif_data_new_from_file(sourcePath.toUtf8().constData());
    if (!ed) {
        image.save(path);
        return;
    }
    const ExifByteOrder bo = exif_data_get_byte_order(ed);
    ExifEntry *e = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_X_RESOLUTION);
    if (e)
        exif_set_long(e->data, bo, image.width());
    e = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_Y_RESOLUTION);
    if (e)
        exif_set_long(e->data, bo, image.height());

    e = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_PIXEL_X_DIMENSION);
    if (e)
        exif_set_long(e->data, bo, image.width());
    e = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_PIXEL_Y_DIMENSION);
    if (e)
        exif_set_long(e->data, bo, image.height());

    if (ed->data && !mat.isIdentity()) {
        QImage thumb = QImage::fromData(ed->data, ed->size);
        thumb = thumb.transformed(mat);
        free(ed->data);
        ed->data = 0;
        ed->size = 0;
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if (!thumb.save(&buffer, sourcePath.mid( sourcePath.lastIndexOf('.')+1).toLatin1().constData())) {
            qWarning("save thumbnail error");
        } else {
            ed->data = (unsigned char*)malloc(data.size());
            ed->size = data.size();
            memcpy(ed->data, data.constData(), data.size());
        }
        buffer.close();
    }
    unsigned char *exif_data;
    unsigned int exif_data_len;
    exif_data_save_data(ed, &exif_data, &exif_data_len);
    if (exif_data_len) {
        free(exif_data);
        if (exif_data_len > 0xffff)
            qWarning("Too much exif data");
    }
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        exif_data_unref(ed);
        qWarning() << "Failed to open image: " << f.errorString();
        return;
    }
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, sourcePath.mid( sourcePath.lastIndexOf('.')+1).toLatin1().constData())) {
        qWarning("save image error");
        exif_data_unref(ed);
        return;
    }
    buffer.close();
    JPEGData *jdata = jpeg_data_new();
    jpeg_data_load_data(jdata, (const unsigned char*)data.constData(), data.size());
    jpeg_data_set_exif_data(jdata, ed);

    unsigned char *d = NULL;
    unsigned int s = 0;
    jpeg_data_save_data(jdata, &d, &s);
    f.write((const char*)d, s);
    f.close();
    exif_data_unref(ed);
    jpeg_data_unref(jdata);
}

}
