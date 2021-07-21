#include "imageviewer.h"

#include <QDebug>

class ImageViewerPrivate
{
public:
    ImageViewerPrivate(ImageViewer *parent);

public:
    ImageViewer *q_ptr;
    ImgViewerType m_imgViewerType;
    Q_DECLARE_PUBLIC(ImageViewer)
};

ImageViewerPrivate::ImageViewerPrivate(ImageViewer *parent)
    : q_ptr(parent)
{
}


ImageViewer::ImageViewer(ImgViewerType imgViewerType, QWidget *parent)
    : DWidget(parent)
    , d_ptr(new ImageViewerPrivate(this))
{
    Q_D(ImageViewer);
    d->m_imgViewerType = imgViewerType;
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
