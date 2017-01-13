#ifndef ALBUMBTCONTENT_H
#define ALBUMBTCONTENT_H

#include "widgets/themewidget.h"

class Slider;
class QHBoxLayout;
class QLabel;
class AlbumBTContent : public ThemeWidget
{
    Q_OBJECT
public:
    explicit AlbumBTContent(const QString& darkStyle,
                            const QString& lightStyle,
                            QWidget *parent = 0);
    void updateCount();
    void changeItemSize(bool increase);

    bool inAlbumView() const;
    void setInAlbumView(bool inAlbumView);
    void updateSliderDefaultValue();

    QString album() const;
    void setAlbum(const QString &album);

signals:
    void itemSizeChanged(int size);
    void multipleChanged(int m);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    void initSynchroBtn();
    void initMiddleContent();
    void initSlider();

    void updateColor();

private:
    bool m_inAlbumView;
    Slider *m_slider;
    QHBoxLayout *m_layout;
    QLabel *m_label;

    //the m_tl1Color is the outside border,
    //which is draw by other widgets.
    QColor m_tl2Color;

    QString m_album;
};

#endif // ALBUMBTCONTENT_H
