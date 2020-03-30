#include "module/slideshow/slideshowbottombar.h"
#include "application.h"

DWIDGET_USE_NAMESPACE
namespace  {
    const QSize ICON_SIZE = QSize(50, 50);
    const int ICON_SPACING = 10;
    const int WIDTH = 260;
    const int HEIGHT = 81;

}

SlideShowBottomBar::SlideShowBottomBar(QWidget *parent) : DFloatingWidget(parent)
{
    setCursor(Qt::ArrowCursor);
//    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    setFixedSize(WIDTH, HEIGHT);
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(ICON_SPACING, 0, ICON_SPACING, 0);
//    hb->setSpacing(0);

    m_preButton = new DIconButton(this);
    m_preButton->setObjectName("PreviousButton");
    m_preButton->setFixedSize(ICON_SIZE);
    m_preButton->setIcon(QIcon::fromTheme("dcc_previous_normal"));
    m_preButton->setIconSize(QSize(36,36));
    m_preButton->setToolTip(tr("Previous"));
    hb->addWidget(m_preButton);
    hb->addSpacing(4);

    connect(m_preButton, &DIconButton::clicked, this, [=]{
        emit dApp->signalM->updatePauseButton();
        emit dApp->signalM->updateButton();
        emit showPrevious();
    });

    m_playpauseButton = new DIconButton(this);
    m_playpauseButton->setShortcut(Qt::Key_Space);
    m_playpauseButton->setObjectName("PlayPauseButton");
    m_playpauseButton->setFixedSize(ICON_SIZE);
    m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
    m_playpauseButton->setIconSize(QSize(36,36));
    m_playpauseButton->setToolTip(tr("Pause"));
    hb->addWidget(m_playpauseButton);
    hb->addSpacing(4);

    connect(m_playpauseButton, &DIconButton::clicked, this, [=]{
        if(0 == a){
            m_playpauseButton->setIcon(QIcon::fromTheme("dcc_play_normal"));
            m_playpauseButton->setToolTip(tr("Play"));
            a = 1;
            emit dApp->signalM->updateButton();

        }
        else {
            m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
            m_playpauseButton->setToolTip(tr("Pause"));
            a = 0;
            emit dApp->signalM->sigStartTimer();
        }
    });

    connect(dApp->signalM, &SignalManager::updatePauseButton, this, [=]{
        m_playpauseButton->setIcon(QIcon::fromTheme("dcc_play_normal"));
        m_playpauseButton->setToolTip(tr("Play"));
        a = 1;
    });
    connect(dApp->signalM, &SignalManager::initButton, this, [=]{
        m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
        m_playpauseButton->setToolTip(tr("Pause"));
        a = 0;
    });

    m_nextButton = new DIconButton(this);
    m_nextButton->setObjectName("NextButton");
    m_nextButton->setFixedSize(ICON_SIZE);
    m_nextButton->setIcon(QIcon::fromTheme("dcc_next_normal"));
    m_nextButton->setIconSize(QSize(36,36));
    m_nextButton->setToolTip(tr("Next"));
    hb->addWidget(m_nextButton);
    hb->addSpacing(4);
    connect(m_nextButton, &DIconButton::clicked, this, [=]{
        emit dApp->signalM->updatePauseButton();
        emit dApp->signalM->updateButton();
        emit showNext();
    });

    m_cancelButton = new DIconButton(this);
    m_nextButton->setObjectName("CancelButton");
    m_cancelButton->setFixedSize(ICON_SIZE);
    m_cancelButton->setIcon(QIcon::fromTheme("dcc_exit_normal"));
    m_cancelButton->setIconSize(QSize(36,36));
    m_cancelButton->setToolTip(tr("Exit"));
    hb->addWidget(m_cancelButton);
    connect(m_cancelButton, &DIconButton::clicked, this, [=]{
        emit showCancel();
    });

    setLayout(hb);

}

//void SlideShowBottomBar::playpauseButton(bool a)
//{
//    if(!a){
//        m_playpauseButton->setIcon(QIcon::fromTheme("dcc_play_normal"));
//        m_playpauseButton->setToolTip(tr("Play"));
//        emit dApp->signalM->updateButton();

//    }
//    else {
//        m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
//        m_playpauseButton->setToolTip(tr("Pause"));
//        emit dApp->signalM->sigStartTimer();
//    }
//}


//void SlideShowBottomBar::onThemeChanged(ViewerThemeManager::AppTheme theme) {

//}


