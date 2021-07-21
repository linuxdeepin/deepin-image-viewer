#include "imageviewer.h"

#include <QDebug>

#include "imageengine.h"
#include "modules/viewpanel.h"
#include <QVBoxLayout>
class ImageViewerPrivate
{
public:
    ImageViewerPrivate(ImageViewer *parent);

public:
    ImageViewer *q_ptr;
    ViewPanel *m_panel = nullptr;
    ImgViewerType m_imgViewerType;
    Q_DECLARE_PUBLIC(ImageViewer)
};

ImageViewerPrivate::ImageViewerPrivate(ImageViewer *parent)
    : q_ptr(parent)
{
    Q_Q(ImageViewer);
    qDebug() << "xxx";
    QVBoxLayout *layout = new QVBoxLayout(q);
    m_panel = new ViewPanel(q);
    m_panel->loadImage("/home/lmh/Pictures/12213.jpg");
    layout->addWidget(m_panel, 0, Qt::AlignCenter);

}


ImageViewer::ImageViewer(ImgViewerType imgViewerType, QWidget *parent)
    : DWidget(parent)
    , d_ptr(new ImageViewerPrivate(this))
{
    Q_D(ImageViewer);
    d->m_imgViewerType = imgViewerType;
    ImageEngine::instance()->makeImgThumbnail("", QStringList());
}

ImageViewer::~ImageViewer()
{

}

void ImageViewer::startChooseFileDialog()
{

}

void ImageViewer::startImgView(QString currentPath, QStringList paths)
{
    //展示当前图片
    //初始化工具栏,当前工具栏中缩略图列表图片为空
    //启动线程制作缩略图
}
