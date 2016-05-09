#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QEvent>
#include <QLabel>
#include <dimagebutton.h>

using namespace Dtk::Widget;

class QLabel;
class ImageButton : public DImageButton
{
    Q_OBJECT
public:
    explicit ImageButton(QWidget *parent = 0);

signals:
    void mouseLeave();

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private:
    void initStyleSheet();
    void showTooltip(const QPoint &gPos);

private:
    bool m_tooltipVisiable;
};

#endif // IMAGEBUTTON_H
