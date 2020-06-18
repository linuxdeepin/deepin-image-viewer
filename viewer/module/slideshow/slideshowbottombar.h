#ifndef SLIDESHOWBOTTOMBAR_H
#define SLIDESHOWBOTTOMBAR_H

#include <DIconButton>
#include <DLabel>
#include <QHBoxLayout>
#include <DFloatingWidget>

#include "controller/viewerthememanager.h"
#include "controller/signalmanager.h"

DWIDGET_USE_NAMESPACE

class SlideShowBottomBar : public DFloatingWidget
{
    Q_OBJECT

public:
    explicit SlideShowBottomBar(QWidget *parent = nullptr);

public:
    DIconButton *m_preButton;
    DIconButton *m_nextButton;
    DIconButton *m_playpauseButton;
    DIconButton *m_cancelButton;
    int a = 0;
//    bool playorpause = true;
//    bool a = true;
//    void playpauseButton(bool a);

private slots:
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);

signals:
    void showPrevious();
    void showPause();
    void showNext();
    void showCancel();

};

#endif // SLIDESHOWBOTTOMBAR_H
