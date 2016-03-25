#pragma once

#include "module/modulepanel.h"
#include "module/view/imagewidget.h"
#include <QStackedWidget>

namespace filter2d {
class FilterObj;
}
class FilterSetup;
class CutWidget;
class EditPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit EditPanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setFilterId(int value);
    void setFilterIndensity(qreal value);

private Q_SLOTS:
    void openImage(const QString& path);
    void applyFilter();

private:
    ImageWidget *m_view = NULL;
    FilterSetup* m_filterSetup;
    int m_filterId = 0;
    qreal m_filterIndensity = 0;
    filter2d::FilterObj* m_filter = NULL;
    QString m_path;
    QImage m_image;
    CutWidget *m_cut;
    QStackedWidget *m_stack;
};
