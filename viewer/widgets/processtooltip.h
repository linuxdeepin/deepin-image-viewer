#ifndef PROCESSTOOLTIP_H
#define PROCESSTOOLTIP_H

#include "blureframe.h"

class QLabel;
class ProcessTooltip : public BlureFrame
{
    Q_OBJECT
public:
    explicit ProcessTooltip(QWidget *parent, QWidget *source);
    void showTooltip(const QString &message, bool success);

private:
    QLabel *m_icon;
    QLabel *m_message;
};

#endif // PROCESSTOOLTIP_H
