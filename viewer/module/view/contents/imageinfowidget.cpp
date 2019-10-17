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

const int TITLE_MAXWIDTH = 79;

struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDataBasics[] = {
    {"FileName",            QT_TRANSLATE_NOOP("MetadataName", "图片名称")},
    {"FileFormat",          QT_TRANSLATE_NOOP("MetadataName", "图片类型")},
    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "图片大小")},
    {"Dimension",           QT_TRANSLATE_NOOP("MetadataName", "图片尺寸")},
    {"DateTimeDigitized",   QT_TRANSLATE_NOOP("MetadataName", "修改日期")},
//    {"FileName",            QT_TRANSLATE_NOOP("MetadataName", "Name")},
//    {"FileFormat",          QT_TRANSLATE_NOOP("MetadataName", "Type")},
//    {"DateTimeOriginal",    QT_TRANSLATE_NOOP("MetadataName", "Date captured")},
//    {"DateTimeDigitized",   QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
//    {"Dimension",           QT_TRANSLATE_NOOP("MetadataName", "Dimension")},
//    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "File size")},
    {"", ""}
};

static MetaData MetaDataDetails[] = {
    {"ExposureMode",        QT_TRANSLATE_NOOP("MetadataName", "曝光模式")},
    {"ExposureProgram",     QT_TRANSLATE_NOOP("MetadataName", "曝光程序")},
    {"ExposureTime",        QT_TRANSLATE_NOOP("MetadataName", "曝光时间")},
    {"Flash",               QT_TRANSLATE_NOOP("MetadataName", "闪光灯")},
    {"ApertureValue",       QT_TRANSLATE_NOOP("MetadataName", "光圈大小")},
    {"FocalLength",         QT_TRANSLATE_NOOP("MetadataName", "焦距")},
    {"ISOSpeedRatings",     QT_TRANSLATE_NOOP("MetadataName", "IOS光感度")},
    {"MaxApertureValue",    QT_TRANSLATE_NOOP("MetadataName", "最大光圈值")},
    {"MeteringMode",        QT_TRANSLATE_NOOP("MetadataName", "测光模式")},
    {"WhiteBalance",        QT_TRANSLATE_NOOP("MetadataName", "白平衡")},
    {"FlashExposureComp",   QT_TRANSLATE_NOOP("MetadataName", "闪光灯补偿")},
    {"Model",               QT_TRANSLATE_NOOP("MetadataName", "镜头型号")},
//    {"ColorSpace",          QT_TRANSLATE_NOOP("MetadataName", "Colorspace")},
//    {"ExposureMode",        QT_TRANSLATE_NOOP("MetadataName", "Exposure mode")},
//    {"ExposureProgram",     QT_TRANSLATE_NOOP("MetadataName", "Exposure program")},
//    {"ExposureTime",        QT_TRANSLATE_NOOP("MetadataName", "Exposure time")},
//    {"Flash",               QT_TRANSLATE_NOOP("MetadataName", "Flash")},
//    {"ApertureValue",       QT_TRANSLATE_NOOP("MetadataName", "Aperture")},
//    {"FocalLength",         QT_TRANSLATE_NOOP("MetadataName", "Focal length")},
//    {"ISOSpeedRatings",     QT_TRANSLATE_NOOP("MetadataName", "ISO")},
//    {"MaxApertureValue",    QT_TRANSLATE_NOOP("MetadataName", "Max aperture")},
//    {"MeteringMode",        QT_TRANSLATE_NOOP("MetadataName", "Metering mode")},
//    {"WhiteBalance",        QT_TRANSLATE_NOOP("MetadataName", "White balance")},
//    {"FlashExposureComp",   QT_TRANSLATE_NOOP("MetadataName", "Flash compensation")},
//    {"Model",               QT_TRANSLATE_NOOP("MetadataName", "Camera model")},
//    {"LensType",            QT_TRANSLATE_NOOP("MetadataName", "Lens model")},
    {"", ""}
};

static int maxTitleWidth()
{
    int maxWidth = 0;
    QFont tf;
    tf.setPixelSize(12);
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

class DFMDArrowLineExpand : public DArrowLineExpand{
public:
    DFMDArrowLineExpand(){
        if (headerLine()) {
            QFont f = headerLine()->font();
//            f.setBold(true);
            f.setPixelSize(14);
            headerLine()->setFont(f);
            headerLine()->setLeftMargin(10);
        }
    }
protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        QRectF bgRect;
        bgRect.setSize(size());
        const QPalette pal = QGuiApplication::palette();//this->palette();
        QColor bgColor = pal.color(QPalette::Background);

        QPainterPath path;
        path.addRoundedRect(bgRect, 8, 8);
        // drawbackground color
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, bgColor);
        painter.setRenderHint(QPainter::Antialiasing, false);
    }
};

#include "imageinfowidget.moc"

ImageInfoWidget::ImageInfoWidget(const QString &darkStyle, const QString &lightStyle, QWidget *parent)
    : QFrame( parent),
      m_maxTitleWidth(maxTitleWidth())
{
//    setObjectName("ImageInfoScrollArea");
    setFixedWidth(300);
    setFrameStyle(QFrame::NoFrame);
//    setWidgetResizable(true);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);
//    QPalette palette;
//    palette.setColor(QPalette::Background, QColor(0,0,0,0)); // 最后一项为透明度
//    setPalette(palette);

//    QFrame *content = new QFrame();
//    QVBoxLayout *contentLayout = new QVBoxLayout(content);
//    contentLayout->setContentsMargins(10, 10, 10, 10);

    // Title field
    SimpleFormLabel *title = new SimpleFormLabel(tr("Image info"));
    title->setFixedHeight(50);

    QFont font;
    font.setPixelSize(14);
    title->setFont(font);

    DIconButton *m_close = new DIconButton(this);
    m_close->setIcon(QIcon(":/resources/light/images/close_normal .svg"));
    m_close->setIconSize(QSize(36,36));
    m_close->setFlat(true);
    m_close->move(257,7);
    DPalette palette1 ;
    palette1.setColor(DPalette::Background, QColor(0,0,0,0));
    m_close->setPalette(palette1);

    connect(m_close, &DIconButton::clicked, this, [ = ] {
        emit dApp->signalM->hideExtensionPanel();
    });


//    title->setAlignment(Qt::AlignCenter);
//    contentLayout->addWidget(title);
//    ViewSeparator *separator = new ViewSeparator();
//    contentLayout->addWidget(separator);
//    contentLayout->addSpacing(3);
    // Info field
    m_exif_base = new QFrame(this);
    m_exif_base->setFixedWidth(280);
    m_exif_details = new QFrame(this);
    m_exif_details->setFixedWidth(280);
    m_exifLayout_base = new QFormLayout();
//    m_exifLayout_base->setSpacing(3);
    m_exifLayout_base->setHorizontalSpacing(16);
    m_exifLayout_base->setContentsMargins(10, 0, 7, 11);
    m_exifLayout_base->setLabelAlignment(Qt::AlignLeft);
    m_separator = new ViewSeparator();
    m_separator->setVisible(false);
    m_exifLayout_details = new QFormLayout();
//    m_exifLayout_details->setSpacing(3);
    m_exifLayout_details->setHorizontalSpacing(16);
    m_exifLayout_details->setContentsMargins(10, 0, 7, 11);
    m_exifLayout_details->setLabelAlignment(Qt::AlignLeft);

    m_exif_base->setLayout(m_exifLayout_base);
    m_exif_details->setLayout(m_exifLayout_details);

//    contentLayout->addLayout(m_exifLayout_base);
//    contentLayout->addSpacing(3);
//    contentLayout->addWidget(m_separator);
//    contentLayout->addSpacing(3);
//    contentLayout->addLayout(m_exifLayout_details);

//    contentLayout->addSpacing(35);
//    contentLayout->addStretch();

//    setWidget(content);


    m_mainLayout = new QVBoxLayout;

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(title, 0, Qt::AlignHCenter | Qt::AlignTop);
//    m_mainLayout->addWidget(m_editStackWidget, 0, Qt::AlignHCenter | Qt::AlignTop);


    setLayout(m_mainLayout);

    m_scrollArea = new QScrollArea();
    QPalette palette = m_scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    m_scrollArea->viewport()->setPalette(palette);
    m_scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    QFrame *infoframe= new QFrame;
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(15, 0, 15, 0);
    scrollWidgetLayout->setSpacing(10);
    infoframe->setLayout(scrollWidgetLayout);
    m_scrollArea->setWidget(infoframe);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    QVBoxLayout *scrolllayout = new QVBoxLayout;
    scrolllayout->addWidget(m_scrollArea);
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    m_mainLayout->insertLayout(1, scrolllayout, 1);

//    m_basicInfoFrame = createBasicInfoWidget(fileInfo);

    QStringList titleList;
    titleList << tr("基本信息");
    titleList << tr("详细信息");


    m_expandGroup = addExpandWidget(titleList);
    m_expandGroup.at(0)->setContent(m_exif_base);
    m_expandGroup.at(0)->setExpand(true);
    m_expandGroup.at(1)->setContent(m_exif_details);
    m_expandGroup.at(1)->setExpand(true);
}


void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;
    updateInfo();
//    if (! visibleRegion().isNull()) {
//    }
//    m_expandGroup.at(0)->setContent(m_exif_base);
//    m_expandGroup.at(1)->setContent(m_exif_details);
}

void ImageInfoWidget::resizeEvent(QResizeEvent *e)
{
//    QScrollArea::resizeEvent(e);
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

//    QScrollArea::timerEvent(e);
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

//    SimpleFormLabel *infoTitle = new SimpleFormLabel(tr("基本信息"));
//    infoTitle->setAlignment(Qt::AlignLeft);
//    m_exifLayout_base->addRow(infoTitle);

    QFont font;
    font.setPixelSize(12);

    for (MetaData *i = MetaDataBasics; ! i->key.isEmpty(); i ++) {
        QString value = infos.value(i->key);
        if (value.isEmpty()) continue;

        SimpleFormField *field = new SimpleFormField;

        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setFont(font);
        field->setText(wrapStr(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
        title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        title->setFont(font);

        m_exifLayout_base->addRow(title, field);
    }
}

void ImageInfoWidget::updateDetailsInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_details);

//    SimpleFormLabel *infoTitle = new SimpleFormLabel(tr("详细信息"));
//    infoTitle->setAlignment(Qt::AlignLeft);
//    m_exifLayout_base->addRow(infoTitle);
    QFont font;
    font.setPixelSize(12);

    for (MetaData *i = MetaDataDetails; ! i->key.isEmpty(); i ++) {
        QString value = infos.value(i->key);
        if (value.isEmpty()) continue;

        SimpleFormField *field = new SimpleFormField;
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setFont(font);
        field->setText(wrapStr(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
        title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        title->setFont(font);

        m_exifLayout_details->addRow(title, field);
    }

//    m_separator->setVisible(m_exifLayout_details->count() > 10);
}

QList<DBaseExpand *> ImageInfoWidget::addExpandWidget(const QStringList &titleList)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout());
    QList<DBaseExpand *> group;

    for (const QString &title : titleList) {
        DFMDArrowLineExpand *expand = new DFMDArrowLineExpand;//DArrowLineExpand;
        expand->setTitle(title);
        initExpand(layout, expand);
        group.push_back(expand);
    }

    return group;
}
void ImageInfoWidget::initExpand(QVBoxLayout *layout, DBaseExpand *expand)
{
    expand->setFixedHeight(30);
    QMargins cm = layout->contentsMargins();
    QRect rc = contentsRect();
    expand->setFixedWidth(rc.width()-cm.left()-cm.right());
    layout->addWidget(expand, 0, Qt::AlignTop);

    connect(expand, &DBaseExpand::expandChange, this, &ImageInfoWidget::onExpandChanged);
    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(expand, this);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, hanceedWidget, [=](){
        QRect rc = geometry();
        rc.setHeight(contentHeight()+10*2);
        setGeometry(rc);
    });
}

void ImageInfoWidget::onExpandChanged(const bool &e)
{
    DArrowLineExpand *expand = qobject_cast<DArrowLineExpand *>(sender());
    if (expand) {
        if (e) {
            expand->setSeparatorVisible(false);
        } else {
            QTimer::singleShot(200, expand, [ = ] {
                expand->setSeparatorVisible(true);
            });
        }
    }
}

int ImageInfoWidget::contentHeight() const
{
    int expandsHeight = 0;
    int firstExpandHeight = m_expandGroup.size()>0 ? m_expandGroup.first()->getContent()->height() : -1;
    bool atleastOneExpand = false;
    for (const DBaseExpand* expand : m_expandGroup) {
        expandsHeight += 30 + 10;
        if (expand->expand()) {
            expandsHeight += expand->getContent()->height();
            atleastOneExpand = true;
        }
    }

    if (!atleastOneExpand && firstExpandHeight > 0)
        expandsHeight += firstExpandHeight;

//    return (
//            expandsHeight );
    return ( expandsHeight + 40 );
}
