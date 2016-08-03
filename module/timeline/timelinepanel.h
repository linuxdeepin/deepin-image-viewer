#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include "timelineimageview.h"
#include "module/modulepanel.h"
#include "controller/databasemanager.h"
#include <dslider.h>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QJsonObject>

class PopupMenuManager;
class Slider;
class TimelinePanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit TimelinePanel(QWidget *parent = 0);

    bool isMainPanel() Q_DECL_OVERRIDE;
    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void showPanelEvent(ModulePanel *p) Q_DECL_OVERRIDE;

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

    void initConnection();
    void initPopupMenu();
    void initMainStackWidget();
    void initImagesView();
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
    QLabel *m_countLabel = NULL;
    Slider *m_slider;

    PopupMenuManager *m_popupMenu;
    TimelineImageView *m_view = NULL;
    QStackedWidget *m_mainStack = NULL;
};

#endif // TIMELINEPANEL_H
