#include "../libimage-viewer/image-viewer_global.h"
#include "homepagewidget.h"

#include "accessibility/ac-desktop-define.h"
#include "../libimage-viewer/imageviewer.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QDropEvent>


namespace {
const QSize THUMBNAIL_BORDERSIZE = QSize(130, 130);
const QSize THUMBNAIL_SIZE = QSize(128, 128);
const QString ICON_IMPORT_PHOTO_DARK = ":/dark/images/icon_import_photo dark.svg";
const QString ICON_IMPORT_PHOTO_LIGHT = ":/light/images/icon_import_photo.svg";
const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 26);
const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 15);
}  // namespace
HomePageWidget::HomePageWidget(QWidget *parent)
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
    setAcceptDrops(true);
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
                     this, &HomePageWidget::ThemeChange);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &HomePageWidget::ThemeChange);

    QObject::connect(button, &DSuggestButton::clicked, this, &HomePageWidget::openImageInDialog);

    this->ThemeChange(DGuiApplicationHelper::instance()->themeType());
}

HomePageWidget::~HomePageWidget()
{

}

void HomePageWidget::ThemeChange(DGuiApplicationHelper::ColorType type)
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

void HomePageWidget::openImageInDialog()
{
    emit sigOpenImage();
}


void HomePageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (!checkMimeData(mimeData)) {
        return;
    }
    event->setDropAction(Qt::CopyAction);
    event->accept();
    event->acceptProposedAction();
    DWidget::dragEnterEvent(event);
}

void HomePageWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void HomePageWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    QStringList paths;
    for (QUrl url : urls) {
        //修复style问题，取消了path
        //lmh0901判断是否是图片
        paths << url.toLocalFile();
    }
    emit sigDrogImage(paths);
}

bool HomePageWidget::checkMimeData(const QMimeData *mimeData)
{
    if (!mimeData->hasUrls()) {
        return false;
    }
    QList<QUrl> urlList = mimeData->urls();
    if (1 > urlList.size()) {
        return false;
    }

    bool result = false;

    //遍历URL，只要存在图片就允许拖入
    for (QUrl url : urlList) {
        const QString path = url.toLocalFile();
        QFileInfo fileinfo(path);
        if (fileinfo.isDir()) {
            continue;
        } else {
            QFileInfo info(path);
            QMimeDatabase db;
            QMimeType mt = db.mimeTypeForFile(info.filePath(), QMimeDatabase::MatchContent);
            QMimeType mt1 = db.mimeTypeForFile(info.filePath(), QMimeDatabase::MatchExtension);
            QString str = info.suffix().toLower();
            if (mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
                result = true;
                break;
            }

            continue;
        }
    }

    return result;
}
