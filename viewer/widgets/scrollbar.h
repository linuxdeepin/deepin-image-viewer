#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>

class QPropertyAnimation;
class ScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    explicit ScrollBar(QWidget *parent = 0);
    void stopScroll();

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

private:
    QPropertyAnimation *m_animation;
    double m_speedTime;
    int m_directionFlag;
};

#endif // SCROLLBAR_H
