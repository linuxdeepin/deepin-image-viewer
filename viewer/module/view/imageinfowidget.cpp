#include "imageinfowidget.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "utils/imageutils.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDateTime>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QScrollBar>
#include <QtDebug>

struct MetaData {
    QString key;
    QString name;
};

static MetaData MetaDataBasics[] = {
    {"FileName",            QObject::tr("Name")},
    {"FileFormat",          QObject::tr("Type")},
    {"DateTimeOriginal",    QObject::tr("Date photoed")},
    {"DateTimeDigitized",   QObject::tr("Date modified")},
    {"Resolution",          QObject::tr("Resolution")},
    {"FileSize",            QObject::tr("File size")},
    {"", ""}
};

static MetaData MetaDataDetails[] = {
    {"ColorSpace",          QObject::tr("Colorspace")},
    {"ExposureMode",        QObject::tr("Exposure mode")},
    {"ExposureProgram",     QObject::tr("Exposure program")},
    {"ExposureTime",        QObject::tr("Exposure time")},
    {"Flash",               QObject::tr("Flash")},
    {"ApertureValue",       QObject::tr("Aperture")},
    {"FocalLength",         QObject::tr("Focal length")},
    {"ISOSpeedRatings",     QObject::tr("ISO")},
    {"MaxApertureValue",    QObject::tr("Max aperture")},
    {"MeteringMode",        QObject::tr("Metering mode")},
    {"WhiteBalance",        QObject::tr("White balance")},
    {"FlashExposureComp",   QObject::tr("Flash compensation")},
    {"Model",               QObject::tr("Camera model")},
    {"LensType",            QObject::tr("Lens model")},
    {"", ""}
};

static int maxTitleWidth()
{
    int maxWidth = 0;
    QFont tf;
    tf.setPixelSize(11);
    for (const MetaData* i = MetaDataBasics; ! i->key.isEmpty(); ++i) {
        maxWidth = qMax(maxWidth + 1, utils::base::stringWidth(tf, i->name));
    }
    for (const MetaData* i = MetaDataDetails; ! i->key.isEmpty(); ++i) {
        maxWidth = qMax(maxWidth + 1, utils::base::stringWidth(tf, i->name));
    }

    return maxWidth;
}

class ViewSeparator : public QLabel {
    Q_OBJECT
public:
    explicit ViewSeparator(QWidget *parent = 0) : QLabel(parent) {
        setFixedHeight(1);
    }
};
class SimpleFormLabel : public QLabel {
    Q_OBJECT
public:
    explicit SimpleFormLabel(const QString &t, QWidget *parent = 0)
        : QLabel(t, parent) {}
};

class SimpleFormField : public QLabel {
    Q_OBJECT
public:
    explicit SimpleFormField(QWidget *parent = 0)
        : QLabel(parent)
    {
        setWordWrap(true);
    }
};

#include "imageinfowidget.moc"

ImageInfoWidget::ImageInfoWidget(QWidget *parent)
    : QScrollArea(parent),
      m_maxTitleWidth(maxTitleWidth())
{
    setObjectName("ImageInfoScrollArea");
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    QFrame *content = new QFrame();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(10, 10, 10, 10);

    // Title field
    SimpleFormLabel *title = new SimpleFormLabel(tr("Image info"));
    contentLayout->addWidget(title);
    ViewSeparator *separator = new ViewSeparator();
    contentLayout->addWidget(separator);
    contentLayout->addSpacing(8);
    // Info field
    m_exifLayout_base = new QFormLayout();
    m_exifLayout_base->setSpacing(8);
    m_exifLayout_base->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_base->setLabelAlignment(Qt::AlignRight);
    m_separator = new ViewSeparator();
    m_separator->setVisible(false);
    m_exifLayout_details = new QFormLayout();
    m_exifLayout_details->setSpacing(8);
    m_exifLayout_details->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_details->setLabelAlignment(Qt::AlignRight);

    contentLayout->addLayout(m_exifLayout_base);
    contentLayout->addSpacing(3);
    contentLayout->addWidget(m_separator);
    contentLayout->addSpacing(8);
    contentLayout->addLayout(m_exifLayout_details);

    contentLayout->addSpacing(15);
    contentLayout->addStretch();

    setWidget(content);
}


void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;

    updateInfo();
}

void ImageInfoWidget::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    killTimer(m_updateTid);
    m_updateTid = startTimer(500);
}

void ImageInfoWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_updateTid)
        return;

    updateInfo();
    killTimer(m_updateTid);
    m_updateTid = 0;
}

void ImageInfoWidget::clearLayout(QLayout *layout) {
    QFormLayout *fl = static_cast<QFormLayout *>(layout);
    if (fl) {
        // FIXME fl->rowCount() will always increase
        for (int i = 0; i < fl->rowCount(); i++) {
            QLayoutItem *li = fl->itemAt(i, QFormLayout::LabelRole);
            QLayoutItem *fi = fl->itemAt(i, QFormLayout::FieldRole);
            if (li) {
                if (li->widget()) delete li->widget();
                fl->removeItem(li);
            }
            if (fi) {
                if (fi->widget()) delete fi->widget();
                fl->removeItem(fi);
            }
        }
    }
}
//QSize ImageInfoWidget::sizeHint() const
//{
//    return QSize(m_maxContentWidth, height());
//}

void ImageInfoWidget::updateInfo()
{
    using namespace utils::image;
    using namespace utils::base;
    auto mds = getAllMetaData(m_path);
    // Minus layout margins
    m_maxFieldWidth = width() - m_maxTitleWidth - (10 + 8) * 2;

    updateBaseInfo(mds);
    updateDetailsInfo(mds);
}

void ImageInfoWidget::updateBaseInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_base);

    for (MetaData *i = MetaDataBasics; ! i->key.isEmpty(); i ++) {
        QString value = infos.value(i->key);
        if (value.isEmpty()) continue;

        SimpleFormField *field = new SimpleFormField;
        field->setMaximumWidth(m_maxFieldWidth);
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setText(wrapStr(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(i->name + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(m_maxTitleWidth);
        title->setAlignment(Qt::AlignRight | Qt::AlignTop);

        m_exifLayout_base->addRow(title, field);
    }
}

void ImageInfoWidget::updateDetailsInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_details);

    for (MetaData *i = MetaDataDetails; ! i->key.isEmpty(); i ++) {
        QString value = infos.value(i->key);
        if (value.isEmpty()) continue;

        SimpleFormField *field = new SimpleFormField;
        field->setMaximumWidth(m_maxFieldWidth);
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setText(wrapStr(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(i->name + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(m_maxTitleWidth);
        title->setAlignment(Qt::AlignRight | Qt::AlignTop);

        m_exifLayout_details->addRow(title, field);
    }

    m_separator->setVisible(m_exifLayout_details->count() > 0);
}
