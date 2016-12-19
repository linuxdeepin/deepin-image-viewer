#ifndef ALBUMBTCONTENT_H
#define ALBUMBTCONTENT_H

#include <QWidget>

class Slider;
class QHBoxLayout;
class QLabel;
class AlbumBTContent : public QWidget
{
    Q_OBJECT
public:
    explicit AlbumBTContent(QWidget *parent = 0);
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
    void initImportBtn();
    void initMiddleContent();
    void initSlider();

    void updateColor();

private:
    bool m_inAlbumView;
    Slider *m_slider;
    QHBoxLayout *m_layout;
    QLabel *m_label;

    QColor m_tl1Color;
    QColor m_tl2Color;
    QColor m_blColor;
    QString m_album;
};

#endif // ALBUMBTCONTENT_H
