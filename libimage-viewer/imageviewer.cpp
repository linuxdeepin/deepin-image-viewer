#include "imageviewer.h"

#include <QDebug>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <DFileDialog>

#include "imageengine.h"
#include "viewpanel/viewpanel.h"
//#include "widgets/toptoolbar.h"

const int TOP_TOOLBAR_HEIGHT = 50;

class ImageViewerPrivate
{
public:
    ImageViewerPrivate(ImageViewer *parent);

public:
    ImageViewer     *q_ptr;
    ViewPanel       *m_panel = nullptr;
    ImgViewerType   m_imgViewerType;
    Q_DECLARE_PUBLIC(ImageViewer)
};

ImageViewerPrivate::ImageViewerPrivate(ImageViewer *parent)
    : q_ptr(parent)
{
    Q_Q(ImageViewer);
    qDebug() << "xxx";
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    q->setLayout(layout);
    m_panel = new ViewPanel(q);
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
    Q_D(ImageViewer);
    QString filter = tr("All images");

    filter.append('(');
    filter.append(utils::image::supportedImageFormats().join(" "));
    filter.append(')');

    static QString cfgGroupName = QStringLiteral("General"),
                   cfgLastOpenPath = QStringLiteral("LastOpenPath");
    QString pictureFolder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir existChecker(pictureFolder);
    if (!existChecker.exists()) {
        pictureFolder = QDir::currentPath();
    }
//    pictureFolder =
//        dApp->setter->value(cfgGroupName, cfgLastOpenPath, pictureFolder).toString();
#ifndef USE_TEST
    const QStringList &image_list =
        DFileDialog::getOpenFileNames(this, tr("Open Image"), pictureFolder, filter, nullptr,
                                      DFileDialog::HideNameFilterDetails);
#else
    const QStringList image_list = QStringList(QApplication::applicationDirPath() + "/test/jpg113.jpg");
#endif
    if (image_list.isEmpty())
        return;

    startImgView(image_list.first(), image_list);

//    QFileInfo firstFileInfo(vinfo.path);
//    dApp->setter->setValue(cfgGroupName, cfgLastOpenPath, firstFileInfo.path());
//    d->m_panel->loadImage("/home/zouya/Desktop/图片/桌面测试图片/江南烧酒4k动漫壁纸_彼岸图网.jpg");
}

void ImageViewer::startImgView(QString currentPath, QStringList paths)
{
    Q_D(ImageViewer);
    //展示当前图片
    d->m_panel->loadImage(currentPath);
    //初始化工具栏,当前工具栏中缩略图列表图片为空
    //启动线程制作缩略图
}

void ImageViewer::resizeEvent(QResizeEvent *e)
{
//    Q_D(ImageViewer);
//    if (d->m_topToolbar) {
//        d->m_topToolbar->resize(width(), TOP_TOOLBAR_HEIGHT);

////        emit dApp->signalM->resizeFileName();
////        if (e->oldSize()  != e->size()) {
////            emit m_topToolbar->updateMaxBtn();
////        }

//        if (window()->isFullScreen()) {
//            d->m_topToolbar->setVisible(false);
//        } else {
//            d-> m_topToolbar->setVisible(true);
//        }
//    }
    DWidget::resizeEvent(e);
}
