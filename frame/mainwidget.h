#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFrame>
#include <QStackedWidget>
#include "toptoolbar.h"
#include "bottomtoolbar.h"
#include "extensionpanel.h"
#include "controller/signalmanager.h"
#include "module/album/albumpanel.h"
#include "module/timeline/timelinepanel.h"

class MainWidget : public QFrame
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    void resizeEvent(QResizeEvent *);

private:
    void initPanelStack();
    void initTopToolbar();
    void initBottomToolbar();
    void initExtensionPanel();
    void initStyleSheet();

    //panel
    void initTimelinePanel();
    void initAlbumPanel();

private:
    TopToolbar *m_topToolbar;
    BottomToolbar *m_bottomToolbar;
    ExtensionPanel *m_extensionPanel;
    SignalManager *m_signalManager = SignalManager::instance();
    QStackedWidget *m_panelStack;

    TimelinePanel *m_timelinePanel;
    AlbumPanel *m_albumPanel;
};

#endif // MAINWIDGET_H
