/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
