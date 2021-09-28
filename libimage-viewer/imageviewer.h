#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <DWidget>
#include <DIconButton>

#include "image-viewer_global.h"
#include "viewpanel/viewpanel.h"

class AbstractTopToolbar;

DWIDGET_USE_NAMESPACE


class ImageViewerPrivate;
class IMAGEVIEWERSHARED_EXPORT ImageViewer : public DWidget
{
    Q_OBJECT
public:
    //ImgViewerType:图片展示类型, savePath:缩略图保存位置，customTopToolbar:自定义标题栏，nullptr的时候使用内置方案
    explicit ImageViewer(imageViewerSpace::ImgViewerType imgViewerType, QString savePath, AbstractTopToolbar *customTopToolbar = nullptr, QWidget *parent = nullptr);
    ~ImageViewer() override;

    //调用文件选择窗口
    bool startChooseFileDialog();

    //传入路径加载图片
    bool startdragImage(const QStringList &paths);

    //启动图片展示入口
    void startImgView(QString currentPath, QStringList paths = QStringList());

    //设置topbar的显示和隐藏
    void setTopBarVisible(bool visible);

    //设置Bottomtoolbar的显示和隐藏
    void setBottomtoolbarVisible(bool visible);

    //获得工具栏按钮
    DIconButton *getBottomtoolbarButton(imageViewerSpace::ButtonType type);

    //二次开发接口

    //设置图片显示panel右键菜单的显示和隐藏，false为永久隐藏，true为跟随原有策略显示或隐藏
    void setViewPanelContextMenuItemVisiable(ViewPanel::MenuItemId id, bool visiable);

    //设置下方工具栏按钮的显示和隐藏
    void setButtomToolBarButtonVisiable(imageViewerSpace::ButtonType id, bool visiable);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
private:
    QScopedPointer<ImageViewerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ImageViewer)
};

#endif // IMAGEVIEWER_H
