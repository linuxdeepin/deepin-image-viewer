#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "bottomtoolbar.h"
#include "extensionpanel.h"
#include "toptoolbar.h"
#include "controller/signalmanager.h"
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
    void initTopToolbar();

    void initConnection();
    void initPanelStack(bool manager);
    void initStyleSheet();

private:
    QStringList m_infoShowingList;

    QStackedWidget  *m_panelStack;

    BottomToolbar   *m_bottomToolbar;
    ExtensionPanel  *m_extensionPanel;
    TopToolbar      *m_topToolbar;
};

#endif // MAINWIDGET_H
