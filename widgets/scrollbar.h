#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>

class QPropertyAnimation;
class ScrollBar : public QScrollBar
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
public:
    explicit ScrollBar(QWidget *parent = 0);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    void onValueChange(int value);

private:
    QPropertyAnimation *animation;
    int m_oldValue;
};

#endif // SCROLLBAR_H
