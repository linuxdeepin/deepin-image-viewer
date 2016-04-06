#pragma once

#include "module/modulepanel.h"
#include "imagewidget.h"
#include "imageinfowidget.h"
#include "navigationwidget.h"

class ViewPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit ViewPanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
private Q_SLOTS:
    void openImage(const QString& path);

private:
    ImageWidget *m_view = NULL;
    ImageInfoWidget *m_info = NULL;
    NavigationWidget *m_nav = NULL;
};
