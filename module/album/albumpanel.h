#ifndef ALBUMPANEL_H
#define ALBUMPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include "module/modulepanel.h"

class AlbumPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit AlbumPanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

private:
    void initImportFrame();

private:
    QVBoxLayout *m_mainLayout;
};

#endif // ALBUMPANEL_H
