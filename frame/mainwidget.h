#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFrame>
#include "toptoolbar.h"
#include "bottomtoolbar.h"
#include "signalmanager.h"

class MainWidget : public QFrame
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    void resizeEvent(QResizeEvent *);

private:
    void initCenterContent();
    void initTopToolbar();
    void initBottomToolbar();
    void initStyleSheet();

    void updateTopToolbarPosition();
    void updateBottomToolbarPosition();

private:
    QWidget *m_centerContent;
    TopToolbar *m_topToolbar;
    BottomToolbar *m_bottomToolbar;
    SignalManager *m_signalManager = SignalManager::instance();
};

#endif // MAINWIDGET_H
