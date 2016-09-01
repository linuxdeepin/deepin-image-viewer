#ifndef EXTENSIONPANEL_H
#define EXTENSIONPANEL_H

#include <QHBoxLayout>
#include "widgets/blureframe.h"

class ExtensionPanel : public BlurFrame
{
    Q_OBJECT
public:
    explicit ExtensionPanel(QWidget *parent);
    void setContent(QWidget *content);
    void updateRectWithContent();

protected:
//    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    QWidget *m_content;
    QHBoxLayout *m_contentLayout;
};

#endif // EXTENSIONPANEL_H
