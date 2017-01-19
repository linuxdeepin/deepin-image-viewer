#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>

#include "thumbnailwidget.h"
#include "application.h"
namespace  {
const QSize THUMBNAIL_BORDERSIZE = QSize(168, 168);
const QSize THUMBNAIL_SIZE = QSize(166, 166);
const QString DARK_DEFAULT_THUMBNAIL =
        ":/resources/dark/images/empty_defaultThumbnail.png";
const QString LIGHT_DEFAULT_THUMBNAIL =
        ":/resources/light/images/empty_defaultThumbnail.png";
}

ThumbnailWidget::ThumbnailWidget(const QString &darkFile,
const QString &lightFile, QWidget *parent): ThemeWidget(darkFile, lightFile, parent)
{
    setMouseTracking(true);
    m_thumbnailLabel = new QLabel(this);
    m_thumbnailLabel->setObjectName("ThumbnailLabel");
    m_thumbnailLabel->setFixedSize(THUMBNAIL_BORDERSIZE);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());


    m_tips = new QLabel(this);
    m_tips->setObjectName("ThumbnailTips");
    m_tips->setText(tr("No image files found"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_thumbnailLabel,  0, Qt::AlignCenter);
    layout->addSpacing(15);
    layout->addWidget(m_tips,  0, Qt::AlignCenter);
    layout->addStretch();
    setLayout(layout);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThumbnailWidget::onThemeChanged);
}

void ThumbnailWidget::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_inBorderColor = QColor(255, 255, 255, 13);
         m_defaultImage = QPixmap(DARK_DEFAULT_THUMBNAIL);
    } else {
        m_inBorderColor = QColor(0, 0, 0, 13);
        m_defaultImage = QPixmap(LIGHT_DEFAULT_THUMBNAIL);
    }

    ThemeWidget::onThemeChanged(theme);
    update();
}

void ThumbnailWidget::setThumbnailImage(const QPixmap thumbnail) {
    if (thumbnail.isNull()) {
        if (isDeepMode()) {
            m_defaultImage = QPixmap(DARK_DEFAULT_THUMBNAIL);
        } else {
            m_defaultImage = QPixmap(LIGHT_DEFAULT_THUMBNAIL);
        }
    } else {
        m_defaultImage = thumbnail;
    }
    update();
}

void ThumbnailWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (m_defaultImage.size() != THUMBNAIL_SIZE) {
        m_defaultImage = m_defaultImage.scaled(THUMBNAIL_SIZE,
                         Qt::KeepAspectRatio, Qt::SmoothTransformation);

    }
    QPoint startPoint = mapToParent(QPoint(m_thumbnailLabel->x(),
                                           m_thumbnailLabel->y()));
    QPoint imgStartPoint = QPoint(startPoint.x() + (THUMBNAIL_SIZE.width() -
           m_defaultImage.width())/2 + 1, startPoint.y() + (THUMBNAIL_SIZE.height()
           - m_defaultImage.height())/2 + 1);
    QRect imgRect = QRect(imgStartPoint.x(), imgStartPoint.y(),
                          m_defaultImage.width(), m_defaultImage.height());

    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.drawPixmap(imgRect, m_defaultImage);
    QPen pen;
    pen.setColor(m_inBorderColor);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawRect(imgRect);
}

void ThumbnailWidget::mouseMoveEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit mouseHoverMoved();
}
ThumbnailWidget::~ThumbnailWidget(){
}
