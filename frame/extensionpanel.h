#ifndef EXTENSIONPANEL_H
#define EXTENSIONPANEL_H

#include <QHBoxLayout>
#include "blureframe.h"

class ExtensionPanel : public BlureFrame
{
    Q_OBJECT
public:
    explicit ExtensionPanel(QWidget *parent, QWidget *source);
    void setContent(QWidget *content);

protected:
    void paintEvent(QPaintEvent *);

private:
    QHBoxLayout *m_contentLayout;
};

#endif // EXTENSIONPANEL_H
