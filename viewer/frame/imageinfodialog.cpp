#include "imageinfodialog.h"
#include "utils/imageutils.h"
#include <QFormLayout>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QMap>

using namespace utils::image;

namespace {

struct MetaData {
    QString key;
    QString name;
};

static MetaData MetaDatas[] = {
    {"FileName",            QObject::tr("Name")},
    {"FileFormat",          QObject::tr("Type")},
    {"DateTimeOriginal",    QObject::tr("Date photoed")},
    {"DateTimeDigitized",   QObject::tr("Date modified")},
    {"Resolution",          QObject::tr("Resolution")},
    {"FileSize",            QObject::tr("File size")},
    {"MaxApertureValue",    QObject::tr("Max aperture")},
    {"FocalLength",         QObject::tr("Focal length")},
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
        addInfoPair(i->name + ":", v);
    }
}
