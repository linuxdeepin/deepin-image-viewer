#include "imageinfodialog.h"
#include "utils/imageutils.h"
#include <QApplication>
#include <QFormLayout>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QMap>

using namespace utils::image;

namespace {

static ExifItem exifItems[] = {
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_NAME, "Name"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_TYPE, "Type"},
    {EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, "Date photoed"},
    {EXIF_IFD_0, EXIF_TAG_DATE_TIME, "Date modified"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_RESOLUTION, "Resolution"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_SIZE, "File size"},
    {EXIF_IFD_COUNT, EXIF_TAG_MAX_APERTURE_VALUE, "Max aperture"},
    {EXIF_IFD_COUNT, EXIF_TAG_FOCAL_LENGTH, "Focal length"},
    {EXIF_IFD_COUNT, 0, 0}
};

}

ImageInfoDialog::ImageInfoDialog(QWidget *parent, QWidget *source)
    : BlureInfoFrame(parent, source)
{
    m_thumbnail = new QLabel;
    m_thumbnail->setFixedSize(240, 160);
    setTopContent(m_thumbnail);
    setBlurBackground(false);

    this->setMinimumWidth(250);
//    this->setMinimumHeight(380);
}

void ImageInfoDialog::setPath(const QString &path)
{
    const QPixmap p = utils::image::cutSquareImage(QPixmap(path),
                                                   QSize(240, 160));
    m_thumbnail->setPixmap(p);

    auto ei = utils::image::GetExifFromPath(path, true);
    for (const utils::image::ExifItem* i = exifItems; i->tag; ++i) {
        const QString v = ei.value(i->name);

        QFont textFont;
        QFontMetrics fm(textFont);
        QString fontMetricText = fm.elidedText(v, Qt::ElideNone, 200);
        if (v.isEmpty()) {
            continue;
        }
        addInfoPair(qApp->translate("ExifItemName", i->name) + ":", fontMetricText);
    }
}
