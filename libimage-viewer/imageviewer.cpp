#include "imageviewer.h"

#include <QDebug>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QDir>
#include <QCollator>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QTranslator>

#include <DFileDialog>

#include "imageengine.h"
#include "viewpanel/viewpanel.h"
#include "service/commonservice.h"
#include "unionimage/imageutils.h"
#include "unionimage/baseutils.h"
//#include "widgets/toptoolbar.h"

#define PLUGINTRANSPATH "/usr/share/libimage-viewer/translations"
class ImageViewerPrivate
{
public:
    ImageViewerPrivate(imageViewerSpace::ImgViewerType imgViewerType, QString savePath, ImageViewer *parent);

public:
    ImageViewer     *q_ptr;
    ViewPanel       *m_panel = nullptr;
    imageViewerSpace::ImgViewerType   m_imgViewerType;
    Q_DECLARE_PUBLIC(ImageViewer)
};

ImageViewerPrivate::ImageViewerPrivate(imageViewerSpace::ImgViewerType imgViewerType, QString savePath, ImageViewer *parent)
    : q_ptr(parent)
{
    QDir dir(PLUGINTRANSPATH);
    if (dir.exists()) {
        QDirIterator qmIt(PLUGINTRANSPATH, QStringList() << QString("*%1.qm").arg(QLocale::system().name()), QDir::Files);
        while (qmIt.hasNext()) {
            qmIt.next();
            QFileInfo finfo = qmIt.fileInfo();
            QTranslator *translator = new QTranslator;
            if (translator->load(finfo.baseName(), finfo.absolutePath())) {
                qApp->installTranslator(translator);
            }
        }
    }
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


ImageViewer::ImageViewer(imageViewerSpace::ImgViewerType imgViewerType, QString savePath, QWidget *parent)
    : DWidget(parent)
    , d_ptr(new ImageViewerPrivate(imgViewerType, savePath, this))
{

    Q_D(ImageViewer);
    //1.load translations first


}

ImageViewer::~ImageViewer()
{

}

bool ImageViewer::startChooseFileDialog()
{
    Q_D(ImageViewer);
    return d->m_panel->startChooseFileDialog();
}

bool ImageViewer::startdragImage(const QStringList &paths)
{
    Q_D(ImageViewer);
    return d->m_panel->startdragImage(paths);
}

void ImageViewer::startImgView(QString currentPath, QStringList paths)
{
    Q_D(ImageViewer);
    //展示当前图片
    d->m_panel->loadImage(currentPath, paths);
    //启动线程制作缩略图
    if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeLocal ||
            CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeNull) {
        //首先生成当前图片的缓存
        ImageEngine::instance()->makeImgThumbnail(CommonService::instance()->getImgSavePath(), QStringList(currentPath), 1);
        //看图制作全部缩略图
        ImageEngine::instance()->makeImgThumbnail(CommonService::instance()->getImgSavePath(), paths, paths.size());
    }
}

void ImageViewer::setTopBarVisible(bool visible)
{
    Q_D(ImageViewer);
    if (d->m_panel) {
        d->m_panel->setTopBarVisible(visible);
    }
}

void ImageViewer::setBottomtoolbarVisible(bool visible)
{
    Q_D(ImageViewer);
    if (d->m_panel) {
        d->m_panel->setBottomtoolbarVisible(visible);
    }
}

DIconButton *ImageViewer::getBottomtoolbarButton(imageViewerSpace::ButtonType type)
{
    DIconButton *button = nullptr;
    Q_D(ImageViewer);
    if (d->m_panel) {
        button = d->m_panel->getBottomtoolbarButton(type);
    }
    return button;
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

void ImageViewer::showEvent(QShowEvent *e)
{
    return DWidget::showEvent(e);
}
