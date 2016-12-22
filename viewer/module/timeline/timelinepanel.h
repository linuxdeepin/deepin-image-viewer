#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include "module/modulepanel.h"
#include "controller/viewerthememanager.h"
#include "widgets/importframe.h"

class QLabel;
class QMenu;
class QStackedWidget;
class Slider;
class TimelineFrame;

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
    void initStyleSheet();
    void updateBottomToolbarContent(int count);
    void onImageCountChanged(int count);

    void appendAction(int id, const QString &text, const QString &shortcut="");
    QMenu* createAlbumMenu();
    void onMenuItemClicked(QAction *action);
    void updateMenuContents();
    void rotateImage(const QString &path, int degree);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    QMenu               *m_menu;
    QLabel              *m_countLabel;
    QStackedWidget      *m_mainStack;
    ImportFrame         *m_importFrame;
    Slider              *m_slider;
    TimelineFrame       *m_frame;

    QStringList         m_rotateList;
};

#endif // TIMELINEPANEL_H
