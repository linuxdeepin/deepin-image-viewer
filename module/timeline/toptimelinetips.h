#ifndef TOPTIMELINETIPS_H
#define TOPTIMELINETIPS_H

#include <QLabel>

class TopTimelineTips : public QLabel
{
    Q_OBJECT
public:
    explicit TopTimelineTips(QWidget *parent = 0);
    void setLeftMargin(int v);
};

#endif // TOPTIMELINETIPS_H
