/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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
#include "shortcut.h"
#include "application.h"

#include "controller/configsetter.h"

namespace {

const QString VIEW_GROUP = "SHORTCUTVIEW";
const QString ALBUM_GROUP = "SHORTCUTALBUM";

QString ss(const QString &group, const QString &text, const QString &defaultValue)
{
    return dApp->setter->value(group, text, defaultValue).toString();
}

}  // namespace

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    ShortcutGroup group1;
#ifndef LITE_DIV
    ShortcutGroup group2;
#endif
    ShortcutGroup group3;

    group1.groupName = tr("Image Viewing");

    group3.groupName = tr("Settings");
    //整理代码结构，解决显示界面出现和快捷键不一致问题，是由于配置文件导致，但配置文件无效，改为不使用配置文件
    group1.groupItems << ShortcutItem(tr("Fullscreen"),  "F11")
                      << ShortcutItem(tr("Exit fullscreen"), "Esc")
                      << ShortcutItem(tr("Extract text"), "Alt + O")
                      << ShortcutItem(tr("Slide show"), "F5")
                      << ShortcutItem(tr("Rename"), "F2")
                      << ShortcutItem(tr("Copy"), "Ctrl + C")
                      << ShortcutItem(tr("Delete"), "Delete")
                      << ShortcutItem(tr("Rotate clockwise"), "Ctrl + R")
                      << ShortcutItem(tr("Rotate counterclockwise"),  "Ctrl + Shift + R")
                      << ShortcutItem(tr("Set as wallpaper"), "Ctrl + F9")
                      << ShortcutItem(tr("Display in file manager"), "Alt + D")
                      << ShortcutItem(tr("Image info"), "Ctrl + I")
                      << ShortcutItem(tr("Previous"), "Left")
                      << ShortcutItem(tr("Next"), "Right")
                      << ShortcutItem(tr("Zoom in"), "Ctrl+ '+'")
                      << ShortcutItem(tr("Zoom out"), "Ctrl+ '-'")
                      << ShortcutItem(tr("Open"), "Ctrl+O");

    group3.groupItems << ShortcutItem(tr("Help"), "F1")
                      << ShortcutItem(tr("Display shortcuts"), "Ctrl + Shift + ?");

#ifndef LITE_DIV
    m_shortcutGroups << group1 << group2 << group3;
#else
    m_shortcutGroups << group1 << group3;
#endif

    // convert to json object
    QJsonArray jsonGroups;
    for (auto scg : m_shortcutGroups) {
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName", scg.groupName);
        QJsonArray jsonGroupItems;
        for (auto sci : scg.groupItems) {
            QJsonObject jsonItem;
            jsonItem.insert("name", sci.name);
            jsonItem.insert("value", sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems", jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut", jsonGroups);
}
QString Shortcut::toStr()
{
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
