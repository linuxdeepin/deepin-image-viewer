#include "imageinfowidget.h"
#include <QLabel>
#include <QFormLayout>
#include <QBoxLayout>
#include <QFileInfo>
#include <QDateTime>
#include <libexif/exif-data.h>
#include <QtDebug>

struct exif_item {
    ExifIfd ifd;
    int tag;
    const char* name;
};

static exif_item exif_items[] = {
    {EXIF_IFD_0, EXIF_TAG_MAKE, QT_TR_NOOP("Manufacture")},
    {EXIF_IFD_0, EXIF_TAG_MODEL, QT_TR_NOOP("Model")},

    {EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME, "Date"},
    {EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE, "White balance"},
    {EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE, "Aperture"},
    {EXIF_IFD_EXIF, EXIF_TAG_MAX_APERTURE_VALUE, "Max aperture"},
    {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE, QT_TR_NOOP("Exposure mode")},
    {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME, "Exposure time"},
    {EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE, "Metering mode"},
    {EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS, "ISO"},
    {EXIF_IFD_EXIF, EXIF_TAG_FLASH, "Flash"},
    {EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH, "Focal length"},
    {EXIF_IFD_COUNT, 0, 0}
};

QMap<QString, QString> GetExifFromPath(const QString& path)
{
    QMap<QString, QString> m;
    qDebug() << "read exif from: " << path;
    ExifData *ed = exif_data_new_from_file(path.toUtf8().data());
    if (!ed) {
        qWarning("exif open error");
        return m;
    }
    for (const exif_item* i = exif_items; i->tag; ++i) {
        ExifEntry *e = exif_content_get_entry(ed->ifd[i->ifd], (ExifTag)i->tag);
        if (!e) {
            qWarning("no exif entry: %s", i->name);
            continue;
        }
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        exif_entry_get_value(e, buf, sizeof(buf));
        if (*buf) {
            QString v = QString(buf).trimmed();
            m.insert(i->name, v);
        }
    }
    exif_data_unref(ed);
    return m;
}

ImageInfoWidget::ImageInfoWidget(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("QLabel { color: white; }");
    QVBoxLayout *vb = new QVBoxLayout();
    setLayout(vb);
    vb->addSpacing(40);
    QLabel *title = new QLabel(tr("EXIF Information"));
    vb->addWidget(title);
    QFrame *line = new QFrame();
    line->setStyleSheet("QFrame { background: white; }");
    line->setFixedHeight(1);
    vb->addWidget(line);
    QFormLayout *fl = new QFormLayout();
    fl->setLabelAlignment(Qt::AlignRight);
    vb->addLayout(fl);
    m_name = new QLabel();
    fl->addRow(tr("Name"), m_name);
    m_date = new QLabel();
    fl->addRow(tr("Date"), m_date);
    m_date_modify = new QLabel();
    fl->addRow(tr("Modified"), m_date_modify);
    m_pixels_x = new QLabel();
    fl->addRow(tr("Width"), m_pixels_x);
    m_pixels_y = new QLabel();
    fl->addRow(tr("Height"), m_pixels_y);
    m_size = new QLabel();
    fl->addRow(tr("File size"), m_size);

    m_detail = new QWidget();
    vb->addWidget(m_detail);
    fl = new QFormLayout();
    fl->setLabelAlignment(Qt::AlignRight);
    m_detail->setLayout(fl);
    for (const exif_item* i = exif_items; i->tag; ++i) {
        QLabel *label = new QLabel();
        label->setProperty("exif_entry", QString::fromUtf8(i->name));
        m_item.append(label);
        fl->addRow(tr(i->name), label);
    }
    m_detail->setVisible(false);

    m_detail_btn = new DTextButton(tr("Show more"));
    vb->addWidget(m_detail_btn);
    connect(m_detail_btn, &DTextButton::clicked, [this]() {
        if (m_detail->isVisible()) {
            m_detail_btn->setText(tr("Show details"));
            m_detail->setVisible(false);
        } else {
            m_detail_btn->setText(tr("Show basics"));
            m_detail->setVisible(true);
        }
    });
    line = new QFrame();
    line->setStyleSheet("QFrame { background: white; }");
    line->setFixedHeight(1);
    vb->addWidget(line);

    vb->addStretch();
}

void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;
    QFileInfo fi(path);
    m_name->setText(fi.fileName());
    m_date_modify->setText(fi.lastModified().toString(QString("yyyy-M-d HH:mm")));
    m_size->setText(QString::number(fi.size()));
    QImage img(path);
    m_pixels_x->setText(QString::number(img.width()));
    m_pixels_y->setText(QString::number(img.height()));

    auto exif_info = GetExifFromPath(path);
    foreach (QLabel *item, m_item) {
        const QString key = item->property("exif_entry").toString();
        if (exif_info.contains(key)) {
            item->setText(exif_info.value(key));
            item->setVisible(true);
            exif_info.remove(key);
        } else {
            item->setText(QString());
            item->setVisible(false);
        }
    }
}
