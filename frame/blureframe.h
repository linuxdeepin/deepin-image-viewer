#ifndef BLUREFRAME_H
#define BLUREFRAME_H

#include <QFrame>

class BlureFrame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QPoint pos READ pos WRITE setPos)
public:
    explicit BlureFrame(QWidget *parent, QWidget *source);
    void setSourceWidget(QWidget *source);
    void setCoverBrush(const QBrush &brush);
    void setBlureRadius(int radius);
    void setPos(const QPoint &pos);
    void move(int x, int y);

protected:
    void paintEvent(QPaintEvent *);
    QPixmap getResultPixmap();

private:
    QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0);

private:
    QWidget *m_sourceWidget;
    QBrush m_coverBrush = QBrush(QColor(0, 0, 0, 200));
    int m_blureRadius = 13;
    QPoint m_pos;
};

#endif // BLUREFRAME_H
