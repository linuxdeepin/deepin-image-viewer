#ifndef ICONTOOLTIP_H
#define ICONTOOLTIP_H

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
#include <QHBoxLayout>

class IconTooltip : public QLabel {
    Q_OBJECT
public:
    IconTooltip(QString iconName, QWidget* parent = 0);
    ~IconTooltip();
public slots:
    void setIconName(QString text);
//protected:
//    void paintEvent(QPaintEvent *);
private:
};
#endif // ICONTOOLTIP_H
