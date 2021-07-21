#include "thumbnailwidget.h"
#include "../libimage-viewer/image-viewer_global.h"
#include "../libimage-viewer/imageviewer.h"
#include "accessibility/ac-desktop-define.h"

namespace {
const QSize THUMBNAIL_BORDERSIZE = QSize(130, 130);
const QSize THUMBNAIL_SIZE = QSize(128, 128);
const QString ICON_IMPORT_PHOTO_DARK = ":/dark/images/icon_import_photo dark.svg";
const QString ICON_IMPORT_PHOTO_LIGHT = ":/light/images/icon_import_photo.svg";
const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 26);
const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 15);
}  // namespace
ThumbnailWidget::ThumbnailWidget(QWidget *parent)
    : DWidget(parent)
{
#ifdef OPENACCESSIBLE
    setAccessibleName(Thumbnail_Widget);
    setObjectName(Thumbnail_Widget);
#endif

    this->setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);

    setMouseTracking(true);
    m_thumbnailLabel = new DLabel(this);
    //    m_thumbnailLabel->setObjectName("ThumbnailLabel");
    m_thumbnailLabel->setFixedSize(THUMBNAIL_BORDERSIZE);

    m_tips = new DLabel(this);
    m_tips->setText(tr("Image file not found"));
    DFontSizeManager::instance()->bind(m_tips, DFontSizeManager::T6);
    m_tips->setForegroundRole(DPalette::TextTips);
    m_tips->hide();

    DSuggestButton *button = new DSuggestButton(tr("Open Image"), this);
    button->setFixedWidth(302);
    button->setFixedHeight(36);
#ifdef OPENACCESSIBLE
    m_thumbnailLabel->setObjectName(Thumbnail_Label);
    m_thumbnailLabel->setAccessibleName(Thumbnail_Label);
    m_tips->setObjectName(NOT_FOUND_IMAGE);
    m_tips->setAccessibleName(NOT_FOUND_IMAGE);
    button->setObjectName(OPEN_IMAGE);
    button->setAccessibleName(OPEN_IMAGE);
#endif

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_thumbnailLabel, 0, Qt::AlignCenter);
    layout->addSpacing(9);
    layout->addWidget(m_tips, 0, Qt::AlignCenter);
    layout->addWidget(button, 0, Qt::AlignCenter);
    layout->addStretch();
    setLayout(layout);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
                     this, &ThumbnailWidget::ThemeChange);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ThumbnailWidget::ThemeChange);

    QObject::connect(button, &DSuggestButton::clicked, this, &ThumbnailWidget::openImageInDialog);

    this->ThemeChange(DGuiApplicationHelper::instance()->themeType());
}

ThumbnailWidget::~ThumbnailWidget()
{

}

void ThumbnailWidget::ThemeChange(DGuiApplicationHelper::ColorType type)
{
    //更改主题颜色的信号,图片跟着变化
    if (type == DGuiApplicationHelper::DarkType) {
        m_picString = ICON_IMPORT_PHOTO_DARK;
        m_theme = true;
    } else if (type == DGuiApplicationHelper::LightType) {
        m_picString = ICON_IMPORT_PHOTO_LIGHT;
        m_theme = false;
    }
    QImage tImg(THUMBNAIL_SIZE, QImage::Format_ARGB32);
    tImg.load(m_picString);
    QPixmap logo_pix = QPixmap::fromImage(tImg);
    m_logo = logo_pix;
    if (m_isDefaultThumbnail) {
        m_defaultImage = m_logo;
    }

    if (type == DGuiApplicationHelper::DarkType) {
        m_inBorderColor = DARK_BORDER_COLOR;
        if (m_isDefaultThumbnail)
            m_defaultImage = m_logo;
        m_deepMode = true;
    } else if (type == DGuiApplicationHelper::LightType) {
        m_inBorderColor = LIGHT_BORDER_COLOR;
        if (m_isDefaultThumbnail)
            m_defaultImage = m_logo;
        m_deepMode = false;
    }
    m_thumbnailLabel->setPixmap(logo_pix);
    update();
}

void ThumbnailWidget::openImageInDialog()
{

    ImageViewer *widget = new ImageViewer(ImgViewerType::ImgViewerTypeLocal);
    widget->setMinimumSize(QSize(800, 600));
    widget->show();


}
