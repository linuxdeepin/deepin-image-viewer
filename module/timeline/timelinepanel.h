#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include "timelineimageview.h"
#include "module/modulepanel.h"
#include "controller/signalmanager.h"
#include "controller/databasemanager.h"
#include <dslider.h>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>

using namespace Dtk::Widget;

class TimelinePanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit TimelinePanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

signals:
    void needGotoAlbumPanel();
    void needGotoSearchPanel();

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

private:
    void initMainStackWidget();
    void initImagesView();
    void initStyleSheet();

    void updateBottomToolbarContent();

private:
    QLabel *m_countLabel = NULL;
    DSlider *m_slider = NULL;

    TimelineImageView *m_imagesView = NULL;
    QStackedWidget *m_mainStackWidget = NULL;
    DatabaseManager *m_databaseManager = NULL;
    SignalManager *m_signalManager = SignalManager::instance();
};

#endif // TIMELINEPANEL_H
