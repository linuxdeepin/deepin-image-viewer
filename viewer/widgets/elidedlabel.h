#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    ElidedLabel(QWidget* parent = 0);
    ~ElidedLabel();

    void setText(const QString &text, int leftMargin = 0);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);

private:
    QString m_text;
    int m_leftMargin;
};
#endif // ELIDEDLABEL_H
