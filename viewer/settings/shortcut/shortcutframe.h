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
#ifndef SHORTCUTFRAME_H
#define SHORTCUTFRAME_H

#include <QFrame>

class QVBoxLayout;
class ShortcutFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ShortcutFrame(QWidget *parent);

signals:
    void resetAll();

private:
    void checkShortcut(bool force = false);
    void resetShortcut();
    void initViewShortcut();
    void initAlbumShortcut();
    void initResetButton();
    const QString trLabel(const char *str);

    QMap<QString, QString> viewValues();
    QMap<QString, QString> albumValues();
private:
    QVBoxLayout *m_layout;
};

#endif // SHORTCUTFRAME_H
