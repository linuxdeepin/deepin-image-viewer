#ifndef SLIDESHOWPANEL_H
#define SLIDESHOWPANEL_H

#include "module/modulepanel.h"

#include <QFileSystemWatcher>

class PopupMenuManager;
class QShortcut;
class SlideEffectPlayer;
class SlideShowPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit SlideShowPanel(QWidget *parent = 0);

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
    const QString menuContent();

    void setImage(const QImage &img);
    void startSlideShow(const SignalManager::ViewInfo &vinfo, bool inDB=true);

    void showFullScreen();
    void showNormal();

private:
    int                  m_hideCursorTid;
    int                  m_startTid;
    QShortcut           *m_sEsc;
    SignalManager::ViewInfo m_vinfo;
    PopupMenuManager    *m_menu;
    QImage               m_img;
    SlideEffectPlayer   *m_player;
    bool                 m_isMaximized;
    QFileSystemWatcher  *m_fileSystemMonitor;
};

#endif // SLIDESHOWPANEL_H
