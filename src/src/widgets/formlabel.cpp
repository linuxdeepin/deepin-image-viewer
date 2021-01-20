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
#include "formlabel.h"

SimpleFormLabel::SimpleFormLabel(const QString &t, QWidget *parent)
    : QLbtoDLabel(t, parent)
{
    QFont font;
    font.setPixelSize(12);
    setFont(font);
    setWordWrap(true);
}

void SimpleFormLabel::resizeEvent(QResizeEvent *event)
{
    if (wordWrap() && sizePolicy().verticalPolicy() == QSizePolicy::Minimum) {
        // heightForWidth rely on minimumSize to evaulate, so reset it before
        setMinimumHeight(0);
        // define minimum height
        setMinimumHeight(heightForWidth(width()));
    }
    QLbtoDLabel::resizeEvent(event);
}

SimpleFormField::SimpleFormField(QWidget *parent)
    : QLbtoDLabel(parent)
{
    QFont font;
    font.setPixelSize(12);
    setFont(font);
    //取消内容信息标签换行
   // setWordWrap(true);
}

void SimpleFormField::resizeEvent(QResizeEvent *event)
{
    if (wordWrap() && sizePolicy().verticalPolicy() == QSizePolicy::Minimum) {
        // heightForWidth rely on minimumSize to evaulate, so reset it before
        setMinimumHeight(0);
        // define minimum height
        setMinimumHeight(heightForWidth(width()));
    }
    QLbtoDLabel::resizeEvent(event);
}
