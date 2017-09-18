/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#ifndef TITLEBUTTON_H
#define TITLEBUTTON_H

#include <QWidget>

class TitleButton : public QWidget
{
    Q_OBJECT
public:
    enum SettingID{
        SlideshowSetting,
        SlideshowEffect,
        SlideshowTime,

        ShortcutSetting,
        ShortcutView,
        ShortcutAlbum
    };

    explicit TitleButton(SettingID id, bool bigFont, const QString &title, QWidget *parent = 0);

    SettingID id() const;
    void setId(const SettingID &id);

    bool isActived() const;
    void setIsActived(bool isActived);

signals:
    void clicked(SettingID id);

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    bool m_bigFont;
    bool m_isActived;
    SettingID m_id;
    QString m_title;
};

#endif // TITLEBUTTON_H
