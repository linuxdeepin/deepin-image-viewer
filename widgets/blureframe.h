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
    void moveWithAnimation(int x, int y);

    int getBorderRadius() const;
    void setBorderRadius(int borderRadius);

    int getBorderWidth() const;
    void setBorderWidth(int borderWidth);

    QColor getBorderColor() const;
    void setBorderColor(const QColor &borderColor);
    void setBlurBackground(bool blur = true);
protected:
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *) override;
    QPixmap getBlurePixmap();

private:
    QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0);

private:
    QPoint m_pressPos;
    QWidget *m_sourceWidget;
    QBrush m_coverBrush;
    int m_blureRadius;
    QPoint m_pos;
    int m_borderRadius;
    int m_borderWidth;
    QColor m_borderColor;
    bool m_blur = true; // TODO this property should be move
    bool m_geometryChanging;
    QTimer *m_geometryTimer;
};

#endif // BLUREFRAME_H
