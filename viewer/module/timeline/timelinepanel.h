#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include "module/modulepanel.h"
#include "controller/databasemanager.h"

class QLabel;
class QStackedWidget;
class PopupMenuManager;
class Slider;
class TimelineImageView;
class TimelinePanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit TimelinePanel(QWidget *parent = 0);

    bool isMainPanel() Q_DECL_OVERRIDE;
    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void showPanelEvent(ModulePanel *p) Q_DECL_OVERRIDE;

private:
    void initConnection();
    void initImagesView();
    void initMainStackWidget();
    void initPopupMenu();
    void initShortcut();
    void initStyleSheet();

    void updateBottomToolbarContent(int count);
    void onImageCountChanged(int count);

    QJsonObject createAlbumMenuObj();
    QString createMenuContent();
    void onMenuItemClicked(int menuId, const QString &text);
    void popupDelDialog(const QStringList paths, const QStringList names);
    void rotateImage(const QString &path, int degree);
    void updateMenuContents();

private:
    PopupMenuManager    *m_popupMenu;
    QLabel              *m_countLabel;
    QStackedWidget      *m_mainStack;
    Slider              *m_slider;
    TimelineImageView   *m_view;

    QStringList         m_rotateList;
};

#endif // TIMELINEPANEL_H
