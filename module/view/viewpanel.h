#pragma once

#include "module/modulepanel.h"
#include "imagewidget.h"
#include "imageinfowidget.h"
#include "navigationwidget.h"
#include "controller/databasemanager.h"
#include "widgets/imagebutton.h"

#include <QFileInfo>
#include <QJsonObject>
#include <QStackedWidget>

class QFileSystemWatcher;
class ImageButton;
class SlideEffectPlayer;
class PopupMenuManager;
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
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private Q_SLOTS:
    void onViewImage(const SignalManager::ViewInfo &vinfo);
    void openImage(const QString& path, bool inDB = true);
    void toggleFullScreen();
    bool showPrevious();
    bool showNext();
    void removeCurrentImage();
    void viewOnNewProcess(const QString &path);

private:
    enum MenuItemId {
        IdFullScreen,
        IdStartSlideShow,
        IdAddToAlbum,
        IdExport,
        IdCopy,
        IdMoveToTrash,
        IdRemoveFromTimeline,
        IdRemoveFromAlbum,
        IdEdit,
        IdAddToFavorites,
        IdRemoveFromFavorites,
        IdShowNavigationWindow,
        IdHideNavigationWindow,
        IdRotateClockwise,
        IdRotateCounterclockwise,
        IdLabel,
        IdSetAsWallpaper,
        IdDisplayInFileManager,
        IdImageInfo,
        IdSubMenu,
        IdSeparator
    };

    void initConnect();
    void initFileSystemWatcher();
    void initFloatBtns();
    void initNavigation();
    void initShortcut();
    void initViewContent();
    void initStack();
    void initStyleSheet();

    QString createMenuContent();

    QJsonObject createAlbumMenuObj(bool isRemove);
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void backToLastPanel();
    void onMenuItemClicked(int menuId, const QString &text);
    void showToolbar(bool isTop);
    void showNormal();
    void showFullScreen();
    void updateMenuContent();

    bool mouseContainsByTopToolbar(const QPoint &pos);
    bool mouseContainsByBottomToolbar(const QPoint &pos);
    int imageIndex(const QString &name);

    QFileInfoList getFileInfos(const QString &path);
    QList<DatabaseManager::ImageInfo> getImageInfos(
            const QFileInfoList &infos);
    const QStringList paths() const;

private:
    bool m_isMaximized;

    ImageWidget *m_view = NULL;
    ImageInfoWidget *m_info = NULL;
    NavigationWidget *m_nav = NULL;
    PopupMenuManager *m_popupMenu;
    SignalManager::ViewInfo m_vinfo;
    QStackedWidget *m_stack;
    ImageButton* m_previousBtn;
    ImageButton* m_nextBtn;
    QLabel* m_scaleLabel;

    QList<DatabaseManager::ImageInfo> m_infos;
    QList<DatabaseManager::ImageInfo>::ConstIterator m_current;
};
