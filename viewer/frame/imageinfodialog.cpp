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

struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDatas[] = {
    {"FileName",            QT_TRANSLATE_NOOP("MetadataName", "Name")},
    {"FileFormat",          QT_TRANSLATE_NOOP("MetadataName", "Type")},
    {"DateTimeOriginal",    QT_TRANSLATE_NOOP("MetadataName", "Date photoed")},
    {"DateTimeDigitized",   QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
    {"Resolution",          QT_TRANSLATE_NOOP("MetadataName", "Resolution")},
    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "File size")},
    {"MaxApertureValue",    QT_TRANSLATE_NOOP("MetadataName", "Max aperture")},
    {"FocalLength",         QT_TRANSLATE_NOOP("MetadataName", "Focal length")},
    {"", ""}
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

    auto mds = getAllMetaData(path);

    for (const MetaData* i = MetaDatas; ! i->key.isEmpty(); i ++) {
        QString v = mds.value(i->key);
        if (v.isEmpty()) continue;
        addInfoPair(qApp->translate("MetadataName", i->name) + ":", v);
    }
}
