#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>

class QTimer;
class QPropertyAnimation;
class ScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    explicit ScrollBar(QWidget *parent = 0);
    void stopScroll();
    bool isScrolling() const;

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

private:
    QTimer *m_timer;  // For mark is scrolling
    QPropertyAnimation *m_animation;
    double m_speedTime;
    int m_directionFlag;
    int m_oldScrollStep;
};

#endif // SCROLLBAR_H
