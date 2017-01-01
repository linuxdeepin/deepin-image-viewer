#ifndef SLIDESHOWPREVIEW_H
#define SLIDESHOWPREVIEW_H

#include <QFrame>

class SlideshowPreview : public QFrame
{
    Q_OBJECT
public:
    enum SlideshowEffect {
        Blinds,
        Switcher,
        Slide,
        Circle
    };

    explicit SlideshowPreview(SlideshowEffect effect, QWidget *parent = 0);
    void resetValue();

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    int activedEffectCount() const;
    bool checked() const;
    bool defaultValue() const;
    void setChecked(bool checked);

private:
    SlideshowEffect m_effect;
    int m_currentFrame;
    int m_animationTID;
    bool m_checked;
};

#endif // SLIDESHOWPREVIEW_H
