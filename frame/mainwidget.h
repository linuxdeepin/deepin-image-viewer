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
#include "module/edit/EditPanel.h"
#include "module/view/viewpanel.h"

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
    void initPanelStack();
    void initTopToolbar();
    void initBottomToolbar();
    void initExtensionPanel();
    void initStyleSheet();

    //panel
    void initTimelinePanel();
    void initAlbumPanel();
    void initEditPanel();
    void initViewPanel();

private:
    TopToolbar *m_topToolbar;
    BottomToolbar *m_bottomToolbar;
    ExtensionPanel *m_extensionPanel;
    SignalManager *m_signalManager = SignalManager::instance();
    QStackedWidget *m_panelStack;

    TimelinePanel *m_timelinePanel;
    AlbumPanel *m_albumPanel;
    EditPanel *m_editPanel;
    ViewPanel *m_viewPanel;
};

#endif // MAINWIDGET_H
