#ifndef BTCONTENT_H
#define BTCONTENT_H

#include <QWidget>

class Slider;
class QHBoxLayout;
class QLabel;
class BTContent : public QWidget
{
    Q_OBJECT
public:
    explicit BTContent(QWidget *parent = 0);
    void updateImageCount();
    void changeItemSize(bool increase);

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

    QColor m_tl1Color;
    QColor m_tl2Color;
    QColor m_blColor;
};

#endif // BTCONTENT_H
