#ifndef SEPARATOR_H
#define SEPARATOR_H

#include <QWidget>
#include <QLabel>

class Separator : public QLabel
{
    Q_OBJECT
public:
    explicit Separator(QWidget *parent = 0);
};

#endif // SEPARATOR_H
