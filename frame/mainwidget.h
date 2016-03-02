#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFrame>
#include "toptoolbar.h"
#include "bottomtoolbar.h"
#include "controller/signalmanager.h"
#include "extensionpanel.h"
#include <QStackedWidget>

class MainWidget : public QFrame
{
    Q_OBJECT

public:
    enum Panel {
        PanelTimeline = 0,
        PanelAlbum = 1,
        PanelEdit = 2
    };

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
};

#endif // MAINWIDGET_H
