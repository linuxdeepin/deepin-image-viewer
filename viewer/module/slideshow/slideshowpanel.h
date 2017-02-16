#ifndef SLIDESHOWPANEL_H
#define SLIDESHOWPANEL_H

#include "module/modulepanel.h"
#include "controller/viewerthememanager.h"
#include <QFileSystemWatcher>

class QMenu;
class QShortcut;
class SlideEffectPlayer;
class SlideShowPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit SlideShowPanel(QWidget *parent = 0);

    enum MenuItemId {
        IdStopslideshow,
        IdPlayOrPause,
    };

    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
private:
    void backToLastPanel();

    QImage getFitImage(const QString &path);

    void initeffectPlay();
    void initMenu();
    void initShortcut();
    void initFileSystemMonitor();

    void setImage(const QImage &img);
    void startSlideShow(const SignalManager::ViewInfo &vinfo, bool inDB=true);
    void appendAction(int id, const QString &text, const QString &shortcut);

    void showFullScreen();
    void showNormal();
    void onMenuItemClicked(QAction *action);
    void onThemeChanged(ViewerThemeManager::AppTheme dark);
private:
    int                  m_hideCursorTid;
    int                  m_startTid;
    QShortcut           *m_sEsc;
    SignalManager::ViewInfo m_vinfo;
    QImage               m_img;
    QMenu               *m_menu;
    SlideEffectPlayer   *m_player;
    bool                 m_isMaximized;
    QFileSystemWatcher  *m_fileSystemMonitor;

    QColor               m_bgColor;
};

#endif // SLIDESHOWPANEL_H
