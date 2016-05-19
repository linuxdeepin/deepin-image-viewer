#pragma once

#include "module/modulepanel.h"
#include "imagewidget.h"
#include "imageinfowidget.h"
#include "navigationwidget.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"

#include <QJsonObject>
#include <QKeyEvent>

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


protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void openImage(const QString& path, bool fromOutside = false);
    void toggleFullScreen();
    void showPrevious();
    void showNext();
    void popupDeleteDialog();
private:
    enum MenuItemId {
        IdFullScreen,
        IdStartSlideShow,
        IdAddToAlbum,
        IdExport,
        IdCopy,
        IdDelete,
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

    QString createMenuContent();

    QJsonObject createAlbumMenuObj(bool isRemove);
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId, const QString &text);
    void initConnect();
    void initShortcut();
    void initStyleSheet();

    void updateCollectButton();
    void updateMenuContent();
    void toggleSlideShow();
    void showToolbar(bool isTop);

    bool mouseContainsByTopToolbar(const QPoint &pos);
    bool mouseContainsByBottomToolbar(const QPoint &pos);

    QList<DatabaseManager::ImageInfo> readImageInfosFromDir(const QString &path);

private:
    QString m_albumName;    // 用于判断图片是否从相册模块传递过来
    ImageButton *m_collectButton;
    ImageWidget *m_view = NULL;
    ImageInfoWidget *m_info = NULL;
    NavigationWidget *m_nav = NULL;
    SlideEffectPlayer *m_slide = NULL;
    PopupMenuManager *m_popupMenu;
    SignalManager *m_signalManager;
    DatabaseManager *m_dbManager;

    QList<DatabaseManager::ImageInfo> m_infos;
    QList<DatabaseManager::ImageInfo>::ConstIterator m_current;
};
