#ifndef TTLCONTENT_H
#define TTLCONTENT_H

#include <QWidget>

class TTLContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTLContent(QWidget *parent = 0);

signals:
    void backToMain();
};

#endif // TTLCONTENT_H
