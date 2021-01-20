#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include "controller/viewerthememanager.h"
#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;


class ElidedLabel : public QLbtoDLabel
{
    Q_OBJECT
public:
    ElidedLabel(QWidget* parent = 0);
    ~ElidedLabel();

    void setText(const QString &text, int leftMargin = 0);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);

private:
    QString m_text;
    int m_leftMargin;
    QColor m_textColor;
};
#endif // ELIDEDLABEL_H
