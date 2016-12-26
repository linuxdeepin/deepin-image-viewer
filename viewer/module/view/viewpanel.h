#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include "module/modulepanel.h"
#include "controller/dbmanager.h"
#include "controller/viewerthememanager.h"
#include "anchors.h"

#include <QFileInfo>
#include <QJsonObject>

DWIDGET_USE_NAMESPACE

class ImageButton;
class ImageInfoWidget;
class ImageView;
class ImageWidget;
class NavigationWidget;
class QFileSystemWatcher;
class QLabel;
class QMenu;
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
    const SignalManager::ViewInfo viewInfo() const;

signals:
    void updateCollectButton();
    void imageChanged(const QString &path);
    void viewImageFrom(QString dir);
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
    void initViewContent();
    void popupDelDialog(const QString path);
    void popupPrintDialog(const QString path);

    // Floating component
    void initFloatingComponent();
    void initSwitchButtons();
    void initScaleLabel();
    void initNavigation();

    // Menu control
    void appendAction(int id, const QString &text, const QString &shortcut="");
    QMenu* createAlbumMenu();
    void onMenuItemClicked(QAction *action);
    void updateMenuContent();

    // View control
    void onViewImage(const SignalManager::ViewInfo &vinfo);
    void openImage(const QString& path, bool inDB = true);
    void removeCurrentImage();
    void rotateImage(bool clockWise);
    bool showNext();
    bool showPrevious();

    // Geometry
    void toggleFullScreen();
    void showNormal();
    void showFullScreen();

    void viewOnNewProcess(const QStringList &paths);
    void backToLastPanel();

    int imageIndex(const QString &path);
    QFileInfoList getFileInfos(const QString &path);
    DBImgInfoList getImageInfos(const QFileInfoList &infos);
    const QStringList paths() const;
private slots:
    void resetImageGeometry();
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    void updateLocalImages();
private:
    int m_hideCursorTid;

    bool m_isMaximized;
    ImageView *m_viewB;
    ImageInfoWidget *m_info;
    QFrame *m_emptyFrame=nullptr;
    QMenu *m_menu;
    QStackedWidget *m_stack;

    // Floating component
    Anchors<NavigationWidget> m_nav;

    SignalManager::ViewInfo m_vinfo;
    DBImgInfoList m_infos;
    DBImgInfoList::ConstIterator m_current;
    QString m_currentImageLastDir = "";
};
#endif // VIEWPANEL_H
