/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "imageinfowidget.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "utils/imageutils.h"
#include "widgets/formlabel.h"

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

namespace {

const int TITLE_MAXWIDTH = 100;

struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDataBasics[] = {
    {"FileName",            QT_TRANSLATE_NOOP("MetadataName", "Name")},
    {"FileFormat",          QT_TRANSLATE_NOOP("MetadataName", "Type")},
    {"DateTimeOriginal",    QT_TRANSLATE_NOOP("MetadataName", "Date photoed")},
    {"DateTimeDigitized",   QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
    {"Resolution",          QT_TRANSLATE_NOOP("MetadataName", "Resolution")},
    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "File size")},
    {"", ""}
};

static MetaData MetaDataDetails[] = {
    {"ColorSpace",          QT_TRANSLATE_NOOP("MetadataName", "Colorspace")},
    {"ExposureMode",        QT_TRANSLATE_NOOP("MetadataName", "Exposure mode")},
    {"ExposureProgram",     QT_TRANSLATE_NOOP("MetadataName", "Exposure program")},
    {"ExposureTime",        QT_TRANSLATE_NOOP("MetadataName", "Exposure time")},
    {"Flash",               QT_TRANSLATE_NOOP("MetadataName", "Flash")},
    {"ApertureValue",       QT_TRANSLATE_NOOP("MetadataName", "Aperture")},
    {"FocalLength",         QT_TRANSLATE_NOOP("MetadataName", "Focal length")},
    {"ISOSpeedRatings",     QT_TRANSLATE_NOOP("MetadataName", "ISO")},
    {"MaxApertureValue",    QT_TRANSLATE_NOOP("MetadataName", "Max aperture")},
    {"MeteringMode",        QT_TRANSLATE_NOOP("MetadataName", "Metering mode")},
    {"WhiteBalance",        QT_TRANSLATE_NOOP("MetadataName", "White balance")},
    {"FlashExposureComp",   QT_TRANSLATE_NOOP("MetadataName", "Flash compensation")},
    {"Model",               QT_TRANSLATE_NOOP("MetadataName", "Camera model")},
    {"LensType",            QT_TRANSLATE_NOOP("MetadataName", "Lens model")},
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

}  // namespace

class ViewSeparator : public QLabel {
    Q_OBJECT
public:
    explicit ViewSeparator(QWidget *parent = 0) : QLabel(parent) {
        setFixedHeight(1);
    }
};

#include "imageinfowidget.moc"

ImageInfoWidget::ImageInfoWidget(const QString &darkStyle, const QString &lightStyle, QWidget *parent)
    : ThemeScrollArea(darkStyle, lightStyle, parent),
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
    contentLayout->addSpacing(3);
    // Info field
    m_exifLayout_base = new QFormLayout();
    m_exifLayout_base->setSpacing(3);
    m_exifLayout_base->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_base->setLabelAlignment(Qt::AlignRight);
    m_separator = new ViewSeparator();
    m_separator->setVisible(false);
    m_exifLayout_details = new QFormLayout();
    m_exifLayout_details->setSpacing(3);
    m_exifLayout_details->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_details->setLabelAlignment(Qt::AlignRight);

    contentLayout->addLayout(m_exifLayout_base);
    contentLayout->addSpacing(3);
    contentLayout->addWidget(m_separator);
    contentLayout->addSpacing(3);
    contentLayout->addLayout(m_exifLayout_details);

    contentLayout->addSpacing(35);
    contentLayout->addStretch();

    setWidget(content);
}


void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;
    updateInfo();
//    if (! visibleRegion().isNull()) {
//    }
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

    QScrollArea::timerEvent(e);
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
const QString ImageInfoWidget::trLabel(const char *str)
{
    return qApp->translate("MetadataName", str);
}

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

        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setText(wrapStr(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
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
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setText(wrapStr(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
        title->setAlignment(Qt::AlignRight | Qt::AlignTop);

        m_exifLayout_details->addRow(title, field);
    }

    m_separator->setVisible(m_exifLayout_details->count() > 0);
}
