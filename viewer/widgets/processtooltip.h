#ifndef PROCESSTOOLTIP_H
#define PROCESSTOOLTIP_H

#include "blureframe.h"
#include "application.h"
#include "controller/viewerthememanager.h"

class QLabel;
class ProcessTooltip : public BlurFrame
{
    Q_OBJECT
public:
    explicit ProcessTooltip(QWidget *parent);
    void showTooltip(const QString &message, bool success);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QLabel *m_icon;
    QLabel *m_message;

    QColor m_coverColor;
    QColor m_borderColor;
    QString m_textColor;
};

#endif // PROCESSTOOLTIP_H
