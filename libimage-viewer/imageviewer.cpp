#include "imageviewer.h"

#include <QDebug>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <DFileDialog>
#include <QDir>
#include <QCollator>

#include "imageengine.h"
#include "viewpanel/viewpanel.h"
#include "service/commonservice.h"
#include "unionimage/imageutils.h"
#include "unionimage/baseutils.h"
//#include "widgets/toptoolbar.h"

const int TOP_TOOLBAR_HEIGHT = 50;

bool compareByFileInfo(const QFileInfo &str1, const QFileInfo &str2)
{
    static QCollator sortCollator;
    sortCollator.setNumericMode(true);
    return sortCollator.compare(str1.baseName(), str2.baseName()) < 0;
}

class ImageViewerPrivate
{
public:
    ImageViewerPrivate(ImgViewerType imgViewerType, QString savePath, ImageViewer *parent);

public:
    ImageViewer     *q_ptr;
    ViewPanel       *m_panel = nullptr;
    ImgViewerType   m_imgViewerType;
    Q_DECLARE_PUBLIC(ImageViewer)
};

ImageViewerPrivate::ImageViewerPrivate(ImgViewerType imgViewerType, QString savePath, ImageViewer *parent)
    : q_ptr(parent)
{
    Q_Q(ImageViewer);
    m_imgViewerType = imgViewerType;
    //记录当前展示模式
    CommonService::instance()->setImgViewerType(imgViewerType);
    //记录缩略图保存路径
    CommonService::instance()->setImgSavePath(savePath);

    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    q->setLayout(layout);
    m_panel = new ViewPanel(q);
    layout->addWidget(m_panel);
}


ImageViewer::ImageViewer(ImgViewerType imgViewerType, QString savePath, QWidget *parent)
    : DWidget(parent)
    , d_ptr(new ImageViewerPrivate(imgViewerType, savePath, this))
{
    Q_D(ImageViewer);
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
    QStringList image_list =
        DFileDialog::getOpenFileNames(this, tr("Open Image"), pictureFolder, filter, nullptr,
                                      DFileDialog::HideNameFilterDetails);
#else
    const QStringList image_list = QStringList(QApplication::applicationDirPath() + "/test/jpg113.jpg");
#endif
    if (image_list.isEmpty())
        return;

    QString DirPath = image_list.first().left(image_list.first().lastIndexOf("/"));
    QDir _dirinit(DirPath);
    QFileInfoList m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    //修复Ｑt带后缀排序错误的问题
    qSort(m_AllPath.begin(), m_AllPath.end(), compareByFileInfo);

    image_list.clear();
    for (int i = 0; i < m_AllPath.size(); i++) {
        QString path = m_AllPath.at(i).filePath();
        //判断是否图片格式
        if (ImageEngine::instance()->isImage(path)) {
            image_list << path;
        }
    }

    startImgView(image_list.first(), image_list);
}

void ImageViewer::startImgView(QString currentPath, QStringList paths)
{
    Q_D(ImageViewer);
    //展示当前图片
    d->m_panel->loadImage(currentPath, paths);
    //启动线程制作缩略图
    if (CommonService::instance()->getImgViewerType() == ImgViewerTypeLocal) {
        //看图制作全部缩略图
        ImageEngine::instance()->makeImgThumbnail(CommonService::instance()->getImgSavePath(), paths, paths.size());
    }
}

void ImageViewer::resizeEvent(QResizeEvent *e)
{
    qDebug() << "ImageViewer::resizeEvent = " << e->size();
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
