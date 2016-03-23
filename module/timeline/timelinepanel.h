#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include <QDropEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "timelineimageview.h"
#include "module/modulepanel.h"
#include "controller/signalmanager.h"
#include "controller/databasemanager.h"

class TimelinePanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit TimelinePanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

private:
    void initMainStackWidget();
    void initImportFrame();
    void initImagesView();
    void importImages();

private:
    TimelineImageView *m_imagesView = NULL;
    QWidget *m_importWidget = NULL;
    QStackedWidget *m_mainStackWidget = NULL;
    DatabaseManager *m_databaseManager = NULL;
    SignalManager *m_signalManager = SignalManager::instance();
};

#endif // TIMELINEPANEL_H
