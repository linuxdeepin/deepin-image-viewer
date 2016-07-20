#pragma once

#include "module/modulepanel.h"
#include "imagewidget.h"
#include "imageinfowidget.h"
#include "navigationwidget.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "imagesliderframe.h"

#include <QJsonObject>
#include <QKeyEvent>
#include <QFileInfo>
#include <QStackedWidget>

class ImageButton;
class SlideEffectPlayer;
class PopupMenuManager;
class ViewPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit ViewPanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

signals:
    void updateCollectButton();
    void imageChanged(const QString &name, const QString &path);

protected:
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onViewImage(const QString &path, const QStringList &paths,
                     const QString &album, bool inDB);
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

    void initStack();
    void initSlider();
    void initViewContent();
    void initNavigation();
    void initConnect();
    void initShortcut();
    void initStyleSheet();

    QString createMenuContent();

    QJsonObject createAlbumMenuObj(bool isRemove);
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId, const QString &text);

    void updateMenuContent();
    void showToolbar(bool isTop);
    void showNormal();
    void showFullScreen();

    bool mouseContainsByTopToolbar(const QPoint &pos);
    bool mouseContainsByBottomToolbar(const QPoint &pos);
    int imageIndex(const QString &name);

    DatabaseManager *dbManager() const;
    QFileInfoList getFileInfos(const QString &path);
    QList<DatabaseManager::ImageInfo> getImageInfos(
            const QFileInfoList &infos);
    const QStringList paths() const;

private:
    bool m_inDB;
    bool m_isMaximized;

    QString m_album;    // 用于判断图片是否从相册模块传递过来
    ImageWidget *m_view = NULL;
    ImageInfoWidget *m_info = NULL;
    NavigationWidget *m_nav = NULL;
    ImageSliderFrame *m_scaleSlider = NULL;
    PopupMenuManager *m_popupMenu;
    SignalManager *m_sManager;
    QStackedWidget *m_stack;

    QList<DatabaseManager::ImageInfo> m_infos;
    QList<DatabaseManager::ImageInfo>::ConstIterator m_current;
};
