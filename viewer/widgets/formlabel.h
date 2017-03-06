#ifndef FORMLABEL_H
#define FORMLABEL_H

#include <QLabel>

class SimpleFormLabel : public QLabel {
    Q_OBJECT
public:
    explicit SimpleFormLabel(const QString &t, QWidget *parent = 0);
};

class SimpleFormField : public QLabel {
    Q_OBJECT
public:
    explicit SimpleFormField(QWidget *parent = 0);
protected:
    void resizeEvent(QResizeEvent* event);
};
#endif // FORMLABEL_H
