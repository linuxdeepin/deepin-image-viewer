#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include "timelineimageview.h"
#include "module/modulepanel.h"
#include "controller/signalmanager.h"
#include "controller/databasemanager.h"
#include <dslider.h>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QJsonObject>

class ConfigSetter;
class PopupMenuManager;
class Slider;
class TimelinePanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit TimelinePanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

private:
    enum MenuItemId {
        IdView,
        IdFullScreen,
        IdStartSlideShow,
        IdAddToAlbum,
        IdExport,
        IdCopy,
        IdMoveToTrash,
        IdRemoveFromTimeline,
        IdEdit,
        IdAddToFavorites,
        IdRemoveFromFavorites,
        IdRotateClockwise,
        IdRotateCounterclockwise,
        IdLabel,
        IdSetAsWallpaper,
        IdDisplayInFileManager,
        IdImageInfo,
        IdSubMenu,
        IdSeparator
    };

    void initPopupMenu();
    void initMainStackWidget();
    void initImagesView();
    void initSelectionView();
    void initStyleSheet();

    void updateBottomToolbarContent(int count);
    void updateMenuContents();
    void onMenuItemClicked(int menuId, const QString &text);
    void onImageCountChanged(int count);

    QString createMenuContent();
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    QJsonObject createAlbumMenuObj();
    const DatabaseManager::ImageInfo imageInfo(const QString &name) const;

private:
    QString m_targetAlbum;  // For import images to an album
    QLabel *m_countLabel = NULL;
    Slider *m_slider;

    ConfigSetter *m_setter;
    PopupMenuManager *m_popupMenu;
    TimelineImageView *m_imagesView = NULL;
    TimelineImageView *m_selectionView = NULL;
    QStackedWidget *m_mainStackWidget = NULL;
    DatabaseManager *m_dbManager = NULL;
    SignalManager *m_sManager = SignalManager::instance();
};

#endif // TIMELINEPANEL_H
