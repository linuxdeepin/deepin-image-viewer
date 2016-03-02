#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include <QVBoxLayout>
#include "module/modulepanel.h"

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
    void initImportFrame();

private:
    QVBoxLayout *m_mainLayout;

};

#endif // TIMELINEPANEL_H
