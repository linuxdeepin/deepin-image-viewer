#ifndef SLIDESHOWPANEL_H
#define SLIDESHOWPANEL_H

#include "module/modulepanel.h"

class PopupMenuManager;
class QShortcut;
class QTimer;
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
private:
    void backToLastPanel();

    QImage getFitImage(const QString &path);

    void initeffectPlay();
    void initMenu();
    void initPlayTimer();
    void initShortcut();

    const QString menuContent();

    void setImage(const QImage &img);
    void startSlideShow(ModulePanel *lastPanel, const QStringList &paths,
                        const QString &path);

    void showFullScreen();
    void showNormal();

private:
    QShortcut           *m_sEsc;
    QTimer              *m_timer;
    ModulePanel         *m_lastPanel;
    PopupMenuManager    *m_menu;
    QImage               m_img;
    SlideEffectPlayer   *m_player;
    bool                 m_isMaximized;
};

#endif // SLIDESHOWPANEL_H
