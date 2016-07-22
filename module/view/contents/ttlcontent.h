#ifndef TTLCONTENT_H
#define TTLCONTENT_H

#include <QWidget>

class TTLContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTLContent(bool inDB, QWidget *parent = 0);

signals:
    void clicked();
};

#endif // TTLCONTENT_H
