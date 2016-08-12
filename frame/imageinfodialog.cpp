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
const int MAX_INFO_LENGTH = 6;
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
    using namespace utils::image;
    const QPixmap p = cutSquareImage(QPixmap(path),
                                                   QSize(240, 160));
    m_thumbnail->setPixmap(p);

    auto ei = getExifFromPath(path, true);
    ei.unite(getExifFromPath(path, false));

    for (const ExifItem* i = exifItems; i->tag; ++i) {
        QString v = ei.value(i->name);

        const int dl = v.length();
        if (dl > MAX_INFO_LENGTH) {
            for(int i = 1; i < dl / MAX_INFO_LENGTH; i++) {
                 int n = i * MAX_INFO_LENGTH;
                 v.insert(n, QLatin1String(" "));
             }
        }

        if (v.isEmpty()) {
            continue;
        }
        QFont textFont;
        QFontMetrics fm(textFont);
        QString ft = fm.elidedText(v, Qt::ElideNone, 200);
        addInfoPair(qApp->translate("ExifItemName", i->name) + ":", ft);
    }
}
