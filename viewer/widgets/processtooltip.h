#ifndef PROCESSTOOLTIP_H
#define PROCESSTOOLTIP_H

#include "blureframe.h"

class QLabel;
class ProcessTooltip : public BlurFrame
{
    Q_OBJECT
public:
    explicit ProcessTooltip(QWidget *parent);
    void showTooltip(const QString &message, bool success);

private:
    QLabel *m_icon;
    QLabel *m_message;
};

#endif // PROCESSTOOLTIP_H
