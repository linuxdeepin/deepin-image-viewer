#include "stackwidget.h"

#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>

StackWidget::StackWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    thumbnailWidget = new ThumbnailWidget(this);
    this->addWidget(thumbnailWidget);

    connect(thumbnailWidget, &ThumbnailWidget::sigOpenImage, this, &StackWidget::slotOpenImage);
    m_viewer = new ImageViewer(ImgViewerTypeNull, this);
    this->addWidget(m_viewer);
}

StackWidget::~StackWidget()
{

}

void StackWidget::slotOpenImage()
{
    setCurrentIndex(1);
}
