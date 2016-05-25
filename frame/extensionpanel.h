#ifndef EXTENSIONPANEL_H
#define EXTENSIONPANEL_H

#include <QHBoxLayout>
#include "widgets/blureframe.h"

class ExtensionPanel : public BlureFrame
{
    Q_OBJECT
public:
    explicit ExtensionPanel(QWidget *parent, QWidget *source);
    void setContent(QWidget *content);
    void updateRectWithContent();

protected:
    void paintEvent(QPaintEvent *);

private:
    QWidget *m_content;
    QHBoxLayout *m_contentLayout;
};

#endif // EXTENSIONPANEL_H
