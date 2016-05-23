#ifndef TTMCONTENT_H
#define TTMCONTENT_H

#include <QWidget>

class TTMContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTMContent(QWidget *parent = 0);

signals:
    void rotateCounterclockwise();
    void rotateClockWise();
    void resetTransform(bool fitWindow);
};

#endif // TTMCONTENT_H
