#ifndef TIMELINEBTCONTENT_H
#define TIMELINEBTCONTENT_H

#include <QWidget>

class Slider;
class QHBoxLayout;
class QLabel;
class TimelineBTContent : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineBTContent(QWidget *parent = 0);
    void updateImageCount();
    void changeItemSize(bool increase);
    int iconSize() const;

signals:
    void itemSizeChanged(int size);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    void initImportBtn();
    void initMiddleContent();
    void initSlider();

    void updateColor();

private:
    Slider *m_slider;
    QHBoxLayout *m_layout;
    QLabel *m_label;

    //the m_tl1Color is the outside border,
    //which is draw by other widgets.
    QColor m_tl2Color;
};

#endif // TIMELINEBTCONTENT_H
