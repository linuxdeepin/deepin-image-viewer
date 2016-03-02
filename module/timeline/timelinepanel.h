#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "module/modulepanel.h"
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

private:
    void initMainStackWidget();
    void initImportFrame();
    void initImagesView();

private:
    QWidget *m_imagesView = NULL;
    QWidget *m_importWidget = NULL;
    QWidget *m_tBottomContent = NULL;
    QWidget *m_tTopleftContent = NULL;
    QWidget *m_tTopMiddleContent = NULL;
    QStackedWidget *m_mainStackWidget = NULL;
    DatabaseManager *m_databaseManager = NULL;
};

#endif // TIMELINEPANEL_H
