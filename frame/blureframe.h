#ifndef BLUREFRAME_H
#define BLUREFRAME_H

#include <QFrame>

class BlureFrame : public QFrame
{
    Q_OBJECT
public:
    explicit BlureFrame(QWidget *parent, QWidget *source);
    void setSourceWidget(QWidget *source);
    void setCoverBrush(const QBrush &brush);
    void setBlureRadius(int radius);

protected:
    void paintEvent(QPaintEvent *);

private:
    QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0);

private:
    QWidget *m_sourceWidget;
    QBrush m_coverBrush = QBrush(QColor(0, 0, 0, 200));
    int m_blureRadius = 13;
};

#endif // BLUREFRAME_H
