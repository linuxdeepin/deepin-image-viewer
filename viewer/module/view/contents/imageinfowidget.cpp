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

#include <DApplicationHelper>
#include <DDialogCloseButton>
#include <DFontSizeManager>
#include <QApplication>
#include <QBoxLayout>
#include <QDateTime>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QString>
#include <QtDebug>

namespace {

const int TITLE_MAXWIDTH = 72 - 10;
const QString ICON_CLOSE_DARK = ":/resources/dark/images/close_normal.svg";
const QString ICON_CLOSE_LIGHT = ":/resources/light/images/close_normal .svg";

#define ArrowLineExpand_HIGHT 30
#define ArrowLineExpand_SPACING 10
#define DIALOG_TITLEBAR_HEIGHT 70

struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDataBasics[] = {
    {"FileName", QT_TRANSLATE_NOOP("MetadataName", "Name")},
    {"DateTimeOriginal", QT_TRANSLATE_NOOP("MetadataName", "Date captured")},
    {"DateTimeDigitized", QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
    {"FileFormat", QT_TRANSLATE_NOOP("MetadataName", "Type")},
    {"Dimension", QT_TRANSLATE_NOOP("MetadataName", "Dimensions")},
    {"FileSize", QT_TRANSLATE_NOOP("MetadataName", "File size")},
    {"Tag", QT_TRANSLATE_NOOP("MetadataName", "Tag")},
    {"", ""}};

static MetaData MetaDataDetails[] = {
    {"ColorSpace", QT_TRANSLATE_NOOP("MetadataName", "Colorspace")},
    {"ExposureMode", QT_TRANSLATE_NOOP("MetadataName", "Exposure mode")},
    {"ExposureProgram", QT_TRANSLATE_NOOP("MetadataName", "Exposure program")},
    {"ExposureTime", QT_TRANSLATE_NOOP("MetadataName", "Exposure time")},
    {"Flash", QT_TRANSLATE_NOOP("MetadataName", "Flash")},
    {"ApertureValue", QT_TRANSLATE_NOOP("MetadataName", "Aperture")},
    {"FocalLength", QT_TRANSLATE_NOOP("MetadataName", "Focal length")},
    {"ISOSpeedRatings", QT_TRANSLATE_NOOP("MetadataName", "ISO")},
    {"MaxApertureValue", QT_TRANSLATE_NOOP("MetadataName", "Max aperture")},
    {"MeteringMode", QT_TRANSLATE_NOOP("MetadataName", "Metering mode")},
    {"WhiteBalance", QT_TRANSLATE_NOOP("MetadataName", "White balance")},
    {"FlashExposureComp", QT_TRANSLATE_NOOP("MetadataName", "Flash compensation")},
    {"Model", QT_TRANSLATE_NOOP("MetadataName", "Camera model")},
    {"LensType", QT_TRANSLATE_NOOP("MetadataName", "Lens model")},
    {"", ""}};

static int maxTitleWidth()
{
    int maxWidth = 0;
    for (const MetaData *i = MetaDataBasics; !i->key.isEmpty(); ++i) {
        maxWidth = qMax(maxWidth + 1,
                        utils::base::stringWidth(
                            DFontSizeManager::instance()->get(DFontSizeManager::T8), i->name));
    }
    for (const MetaData *i = MetaDataDetails; !i->key.isEmpty(); ++i) {
        maxWidth = qMax(maxWidth + 1,
                        utils::base::stringWidth(
                            DFontSizeManager::instance()->get(DFontSizeManager::T8), i->name));
    }

    return maxWidth;
}

}  // namespace

class ViewSeparator : public QLabel
{
    Q_OBJECT
public:
    explicit ViewSeparator(QWidget *parent = 0)
        : QLabel(parent)
    {
        setFixedHeight(1);
    }
};

class DFMDArrowLineExpand : public DArrowLineExpand
{
public:
    DFMDArrowLineExpand()
    {
        if (headerLine()) {
            DFontSizeManager::instance()->bind(headerLine(), DFontSizeManager::T6);

            DPalette pa = DApplicationHelper::instance()->palette(headerLine());
            pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
            headerLine()->setPalette(pa);

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
        const QPalette pal = QGuiApplication::palette();  // this->palette();
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

/**
 ***************************WARNING********************************
 * this is method is a wrong way to implement this property dialog;
 * TODO:: make class Inherits DDialog
 *        then set property infomations to display.
 */

ImageInfoWidget::ImageInfoWidget(const QString &darkStyle, const QString &lightStyle,
                                 QWidget *parent)
    : QFrame(parent)
    , m_maxTitleWidth(maxTitleWidth())
{
    setFixedWidth(300);
    //    setMaximumHeight(300);
    setFrameStyle(QFrame::NoFrame);

    // Title field
    //    SimpleFormLabel *title = new SimpleFormLabel(tr("Image info"));
    //    title->setFixedHeight(50);
    //    DFontSizeManager::instance()->bind(title, DFontSizeManager::T6);

    //    DPalette pa = DApplicationHelper::instance()->palette(title);
    //    pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
    //    title->setPalette(pa);

    // Info field
    m_exif_base = new QFrame(this);
    m_exif_base->setFixedWidth(280);

    m_exif_details = new QFrame(this);
    m_exif_details->setFixedWidth(280);

    m_exifLayout_base = new QFormLayout();
    m_exifLayout_base->setVerticalSpacing(7);
    m_exifLayout_base->setHorizontalSpacing(16);
    m_exifLayout_base->setContentsMargins(10, 1, 7, 10);
    m_exifLayout_base->setLabelAlignment(Qt::AlignLeft);

    m_exifLayout_details = new QFormLayout();
    m_exifLayout_details->setVerticalSpacing(7);
    m_exifLayout_details->setHorizontalSpacing(16);
    m_exifLayout_details->setContentsMargins(10, 1, 7, 10);
    m_exifLayout_details->setLabelAlignment(Qt::AlignLeft);

    m_exif_base->setLayout(m_exifLayout_base);
    m_exif_details->setLayout(m_exifLayout_details);

    m_mainLayout = new QVBoxLayout;

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    m_scrollArea = new QScrollArea();
    QPalette palette = m_scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    m_scrollArea->viewport()->setPalette(palette);
    m_scrollArea->setFrameShape(QFrame::Shape::NoFrame);

    QWidget *scrollContentWidget = new QWidget;
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(0, 0, 10, 0);
    scrollWidgetLayout->setSpacing(ArrowLineExpand_SPACING);
    scrollContentWidget->setLayout(scrollWidgetLayout);
    m_scrollArea->setWidget(scrollContentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    m_mainLayout->addWidget(m_scrollArea, 1);
    this->setLayout(m_mainLayout);
    //    QVBoxLayout *scrolllayout = new QVBoxLayout;
    //    scrolllayout->addWidget(m_scrollArea);

    //    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    //    layout->addLayout(scrolllayout, 1);
#if 0
    m_closedString = "";
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::DarkType) {
        m_closedString = ICON_CLOSE_DARK;
    } else {
        m_closedString = ICON_CLOSE_LIGHT;
    }

    DDialogCloseButton *m_close = new DDialogCloseButton(this);
    m_close->setIcon(QIcon(m_closedString));

    m_close->setIconSize(QSize(36, 36));
    m_close->setFlat(true);
    m_close->move(257, 7);
    DPalette palette1;
    palette1.setColor(DPalette::Background, QColor(0, 0, 0, 1));
    m_close->setPalette(palette1);

    connect(m_close, &DDialogCloseButton::clicked, this,
            [=] { emit dApp->signalM->hideExtensionPanel(); });
#endif
}

void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;
    m_isBaseInfo = false;
    m_isDetailsInfo = false;
    updateInfo();

    QStringList titleList;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout());
    // clear old expandwidget
    if (nullptr != layout) {
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->setParent(nullptr);
            }
            delete child;
        }
    }

    m_exif_base->setParent(this);
    m_exif_details->setParent(this);
    qDeleteAll(m_expandGroup);

    m_expandGroup.clear();

    if (m_isBaseInfo == true && m_isDetailsInfo == true) {
        titleList << tr("Basic info");
        titleList << tr("Details");
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_exif_base);
        m_expandGroup.at(0)->setExpand(true);
        m_expandGroup.at(1)->setContent(m_exif_details);
        m_expandGroup.at(1)->setExpand(true);

    } else if (m_isBaseInfo == false && m_isDetailsInfo == true) {
        titleList << tr("Details");
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_exif_details);
        m_expandGroup.at(0)->setExpand(true);
    } else if (m_isBaseInfo == true && m_isDetailsInfo == false) {
        titleList << tr("Basic info");
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_exif_base);
        m_expandGroup.at(0)->setExpand(true);
    }

    for (auto i = 0; i < m_expandGroup.count(); ++i) {
        layout->addWidget(m_expandGroup.at(i));
    }

    layout->addStretch();
}

void ImageInfoWidget::resizeEvent(QResizeEvent *e)
{
    //    QScrollArea::resizeEvent(e);
    //    killTimer(m_updateTid);
    //    m_updateTid = startTimer(500);

    DWidget::resizeEvent(e);
}

void ImageInfoWidget::timerEvent(QTimerEvent *e)
{
    //    if (e->timerId() != m_updateTid)
    //        return;

    //    updateInfo();
    //    killTimer(m_updateTid);
    //    m_updateTid = 0;

    QWidget::timerEvent(e);

    //    QScrollArea::timerEvent(e);
}

void ImageInfoWidget::clearLayout(QLayout *layout)
{
    QFormLayout *fl = static_cast<QFormLayout *>(layout);
    if (fl) {
        // FIXME fl->rowCount() will always increase
        for (int i = 0; i < fl->rowCount(); i++) {
            QLayoutItem *li = fl->itemAt(i, QFormLayout::LabelRole);
            QLayoutItem *fi = fl->itemAt(i, QFormLayout::FieldRole);
            if (li) {
                if (li->widget())
                    delete li->widget();
                fl->removeItem(li);
            }
            if (fi) {
                if (fi->widget())
                    delete fi->widget();
                fl->removeItem(fi);
            }
        }
    }
}
// QSize ImageInfoWidget::sizeHint() const
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
    //    m_maxFieldWidth = width() - m_maxTitleWidth - 20*2;
    m_maxFieldWidth = width() - TITLE_MAXWIDTH - 20 * 2 - 10 * 2;

    updateBaseInfo(mds);
    updateDetailsInfo(mds);
}

void ImageInfoWidget::updateBaseInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_base);

    for (MetaData *i = MetaDataBasics; !i->key.isEmpty(); i++) {
        QString value = infos.value(i->key);
        if (value.isEmpty())
            continue;

        m_isBaseInfo = true;

        SimpleFormField *field = new SimpleFormField;
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(field, DFontSizeManager::T8);
        DPalette pa1 = DApplicationHelper::instance()->palette(field);
        pa1.setBrush(DPalette::Text, pa1.color(DPalette::TextTitle));
        field->setPalette(pa1);
        field->setText(SpliteText(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        //        title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
        title->setFixedWidth(TITLE_MAXWIDTH);
        title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T8);
        DPalette pa2 = DApplicationHelper::instance()->palette(title);
        pa2.setBrush(DPalette::Text, pa2.color(DPalette::TextTitle));
        title->setPalette(pa2);
        title->setText(SpliteText(trLabel(i->name) + ":", title->font(), TITLE_MAXWIDTH));

        m_exifLayout_base->addRow(title, field);
    }
}

void ImageInfoWidget::updateDetailsInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_details);

    for (MetaData *i = MetaDataDetails; !i->key.isEmpty(); i++) {
        QString value = infos.value(i->key);
        if (value.isEmpty())
            continue;

        m_isDetailsInfo = true;

        SimpleFormField *field = new SimpleFormField;
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(field, DFontSizeManager::T8);
        DPalette pa1 = DApplicationHelper::instance()->palette(field);
        pa1.setBrush(DPalette::Text, pa1.color(DPalette::TextTitle));
        field->setPalette(pa1);
        field->setText(SpliteText(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        title->setFixedWidth(TITLE_MAXWIDTH);
        title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T8);
        DPalette pa2 = DApplicationHelper::instance()->palette(title);
        pa2.setBrush(DPalette::Text, pa2.color(DPalette::TextTitle));
        title->setPalette(pa2);
        title->setText(SpliteText(trLabel(i->name) + ":", title->font(), TITLE_MAXWIDTH));

        m_exifLayout_details->addRow(title, field);
    }
}

QList<DBaseExpand *> ImageInfoWidget::addExpandWidget(const QStringList &titleList)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout());
    QList<DBaseExpand *> group;

    for (const QString &title : titleList) {
        DFMDArrowLineExpand *expand = new DFMDArrowLineExpand;  // DArrowLineExpand;
        expand->setTitle(title);
        initExpand(layout, expand);
        group.push_back(expand);
    }

    return group;
}
void ImageInfoWidget::initExpand(QVBoxLayout *layout, DBaseExpand *expand)
{
    expand->setFixedHeight(ArrowLineExpand_HIGHT);
    QMargins cm = layout->contentsMargins();
    QRect rc = contentsRect();
    expand->setFixedWidth(rc.width() - cm.left() - cm.right());
    expand->setExpandedSeparatorVisible(false);
    expand->setSeparatorVisible(false);
    layout->addWidget(expand, 0, Qt::AlignTop);

    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(expand, expand);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, hanceedWidget, [=]() {
        QRect rc = geometry();
        rc.setHeight(contentHeight() + ArrowLineExpand_SPACING * 2);
        setGeometry(rc);

        emit dApp->signalM->extensionPanelHeight(contentHeight() /*+ ArrowLineExpand_SPACING * 2*/);
    });
}

void ImageInfoWidget::onExpandChanged(const bool &e)
{
    DArrowLineExpand *expand = qobject_cast<DArrowLineExpand *>(sender());
    if (expand) {
        if (e) {
            expand->setSeparatorVisible(false);
        } else {
            QTimer::singleShot(200, expand, [=] { expand->setSeparatorVisible(true); });
        }
    }
}

int ImageInfoWidget::contentHeight() const
{
    int expandsHeight = ArrowLineExpand_SPACING;
    for (const DBaseExpand *expand : m_expandGroup) {
        expandsHeight += expand->height();
    }
    return (DIALOG_TITLEBAR_HEIGHT + expandsHeight + contentsMargins().top() +
            contentsMargins().bottom());
}
