#ifndef BOTTOMTOOLBAR_H
#define BOTTOMTOOLBAR_H

#include "widgets/blureframe.h"
#include "controller/viewerthememanager.h"

#include <QHBoxLayout>


class BottomToolbar : public BlurFrame
{
    Q_OBJECT
public:
    explicit BottomToolbar(QWidget *parent);
    void setContent(QWidget *content);

protected:
    void mouseMoveEvent(QMouseEvent *) override;

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QColor m_coverBrush;
    QHBoxLayout *m_mainLayout;

};

#endif // BOTTOMTOOLBAR_H
