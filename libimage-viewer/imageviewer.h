#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <DWidget>

#include "image-viewer_global.h"

DWIDGET_USE_NAMESPACE

class ImageViewerPrivate;
class IMAGEVIEWERSHARED_EXPORT ImageViewer : public DWidget
{
    Q_OBJECT
public:
    //ImgViewerType:图片展示类型, savePath:缩略图保存位置
    explicit ImageViewer(imageViewerSpace::ImgViewerType imgViewerType, QString savePath, QWidget *parent = nullptr);
    ~ImageViewer() override;

    //调用文件选择窗口
    bool startChooseFileDialog();

    //调用文件选择窗口
    bool startdragImage(const QStringList &paths);

    //启动图片展示入口
    void startImgView(QString currentPath, QStringList paths = QStringList());
protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
private:
    QScopedPointer<ImageViewerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ImageViewer)
};

#endif // IMAGEVIEWER_H
