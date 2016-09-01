#ifndef BLUREFRAME_H
#define BLUREFRAME_H

#include <QFrame>
#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class BlurFrame : public DBlurEffectWidget
{
    Q_OBJECT
    Q_PROPERTY(QPoint pos READ pos WRITE setPos)
public:
    explicit BlurFrame(QWidget *parent);
    void moveWithAnimation(int x, int y);

    QColor  getBorderColor() const;
    int     getBorderRadius() const;
    int     getBorderWidth() const;

    void setBorderColor(const QColor &borderColor);
    void setBorderRadius(int borderRadius);
    void setBorderWidth(int borderWidth);
    void setCoverBrush(const QBrush &brush);
    void setPos(const QPoint &pos);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    QColor  m_borderColor;
    int     m_borderRadius;
    int     m_borderWidth;
    QBrush  m_coverBrush;
};

#endif // BLUREFRAME_H
