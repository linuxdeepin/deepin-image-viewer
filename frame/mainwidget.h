#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "bottomtoolbar.h"
#include "extensionpanel.h"
#include "toptoolbar.h"
#include "controller/signalmanager.h"
#include "module/album/albumpanel.h"
#include "module/edit/EditPanel.h"
#include "module/timeline/timelinepanel.h"
#include "module/view/viewpanel.h"
#include "module/slideshow/slideshowpanel.h"
#include <QFrame>
#include <QStackedWidget>

class MainWidget : public QFrame
{
    Q_OBJECT

public:
    MainWidget(bool manager, QWidget *parent = 0);
    ~MainWidget();

protected:
    void resizeEvent(QResizeEvent *) override;

private slots:
    void onGotoPanel(ModulePanel *panel);
    void onShowProcessTooltip(const QString &message, bool success);
    void onShowImageInfo(const QString &path);

private:
    void initBottomToolbar();
    void initExtensionPanel();
    void initPanelStack();
    void initStyleSheet();
    void initTopToolbar();

    // Panel
    void initAlbumPanel();
    void initEditPanel();
    void initSlideShowPanel();
    void initTimelinePanel();
    void initViewPanel();

private:
    BottomToolbar   *m_bottomToolbar;
    ExtensionPanel  *m_extensionPanel;
    QStackedWidget  *m_panelStack;
    TopToolbar      *m_topToolbar;

    // Panel
    AlbumPanel      *m_albumPanel;
    EditPanel       *m_editPanel;
    SlideShowPanel  *m_slideShowPanel;
    TimelinePanel   *m_timelinePanel;
    ViewPanel       *m_viewPanel;
};

#endif // MAINWIDGET_H
