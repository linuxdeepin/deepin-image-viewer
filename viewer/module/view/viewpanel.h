#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include "module/modulepanel.h"
#include "controller/databasemanager.h"
#include "anchors.h"

#include <QFileInfo>
#include <QJsonObject>

DWIDGET_USE_NAMESPACE

class ImageButton;
class ImageInfoWidget;
class ImageWidget;
class NavigationWidget;
class PopupMenuManager;
class QFileSystemWatcher;
class QLabel;
class QStackedWidget;
class SlideEffectPlayer;

class ViewPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit ViewPanel(QWidget *parent = 0);

    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

signals:
    void updateCollectButton();
    void imageChanged(const QString &path, bool adaptScreen);

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    void initConnect();
    void initFileSystemWatcher();
    void initPopupMenu();
    void initShortcut();
    void initStack();
    void initStyleSheet();
    void initViewContent();

    // Floating component
    void initFloatingComponent();
    void initSwitchButtons();
    void initScaleLabel();
    void initNavigation();

    // Menu control
    const QJsonObject   createAlbumMenuObj(bool isRemove);
    const QString       createMenuContent();
    void                onMenuItemClicked(int menuId, const QString &text);
    void                updateMenuContent();

    // View control
    void onViewImage(const SignalManager::ViewInfo &vinfo);
    void openImage(const QString& path, bool inDB = true);
    void removeCurrentImage();
    void rotateImage(bool clockWise);
    bool showNext();
    bool showPrevious();
    void updateThumbnail(const QString &name);

    // Geometry
    void toggleFullScreen();
    void showNormal();
    void showFullScreen();

    void viewOnNewProcess(const QString &paths);
    void backToLastPanel();

    int imageIndex(const QString &name);
    QFileInfoList getFileInfos(const QString &path);
    QList<DatabaseManager::ImageInfo> getImageInfos(const QFileInfoList &infos);
    const QStringList paths() const;
private slots:
    void resetImageGeometry();

private:
    bool m_isMaximized;
    int m_openTid = 0;

    ImageWidget *m_view;
    ImageInfoWidget *m_info;
    PopupMenuManager *m_popupMenu;
    QStackedWidget *m_stack;

    // Floating component
    Anchors<NavigationWidget> m_nav;

    SignalManager::ViewInfo m_vinfo;
    QList<DatabaseManager::ImageInfo> m_infos;
    QList<DatabaseManager::ImageInfo>::ConstIterator m_current;
};
#endif // VIEWPANEL_H
