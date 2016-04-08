#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QWidget>
#include <QLabel>

class Tooltip : public QLabel
{
    Q_OBJECT
public:
    explicit Tooltip(QWidget *parent = 0);
};

#endif // TOOLTIP_H
