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
#include "application.h"
#include "shortcut.h"

#include "controller/configsetter.h"

namespace {

const QString VIEW_GROUP = "SHORTCUTVIEW";
const QString ALBUM_GROUP = "SHORTCUTALBUM";

QString ss(const QString &group, const QString &text)
{
    return dApp->setter->value(group, text).toString();
}

}  //namespace

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
#ifndef LITE_DIV
    ShortcutGroup group2;
#endif
    ShortcutGroup group3;

    group1.groupName = tr("Image Viewing");
#ifndef LITE_DIV
    group2.groupName = tr("Album");
#endif
    group3.groupName = tr("Settings");
    group1.groupItems<<
#ifndef LITE_DIV
    ShortcutItem(tr("View"), "Enter")<<
#endif
    ShortcutItem(tr("Fullscreen"), ss(VIEW_GROUP, "Fullscreen"))<<
#ifndef LITE_DIV
    ShortcutItem(tr("Slide show"), ss(VIEW_GROUP, "Slide show"))<<
    ShortcutItem(tr("End show"),  ss(VIEW_GROUP, "End show"))<<
#endif
    ShortcutItem(tr("Copy"), ss(VIEW_GROUP, "Copy"))<<
    ShortcutItem(tr("Delete"),  "Delete")<<
#ifndef LITE_DIV
    ShortcutItem(tr("Remove from album"), ss(VIEW_GROUP, "Remove from album"))<<
    ShortcutItem(tr("Favorite"), ss(VIEW_GROUP, "Favorite"))<<
    ShortcutItem(tr("Unfavorite"), ss(VIEW_GROUP, "Unfavorite"))<<
#endif
    ShortcutItem(tr("Rotate clockwise"), ss(VIEW_GROUP, "Rotate clockwise"))<<
    ShortcutItem(tr("Rotate counterclockwise"), ss(VIEW_GROUP, "Rotate counterclockwise"))<<
    ShortcutItem(tr("Set as wallpaper"), ss(VIEW_GROUP, "Set as wallpaper"))<<
    ShortcutItem(tr("Display in file manager"), ss(VIEW_GROUP, "Display in file manager"))<<
    ShortcutItem(tr("Image info"), ss(VIEW_GROUP, "Image info"))<<
    ShortcutItem(tr("Previous"), "Left")<<
    ShortcutItem(tr("Next"), "Right")
                    #ifndef LITE_DIV
                     <<
    ShortcutItem(tr("Previous screen"), "PageUp")<<
    ShortcutItem(tr("Next screen"), "PageDown");
    group2.groupItems<<ShortcutItem(tr("New album"), ss(ALBUM_GROUP, "New album"))<<
                       ShortcutItem(tr("Rename"), ss(ALBUM_GROUP, "Rename"));
#else
                        ;
#endif

    group3.groupItems<<ShortcutItem(tr("Help"),  "F1")<<
                       ShortcutItem(tr("Exit"),  "Ctrl + Q")<<
                       ShortcutItem(tr("Display shortcuts"), "Ctrl + Shift + ?");

#ifndef LITE_DIV
    m_shortcutGroups << group1 << group2 << group3;
#else
    m_shortcutGroups << group1 << group3;
#endif

    //convert to json object
    QJsonArray jsonGroups;
    for(auto scg:m_shortcutGroups){
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName",scg.groupName);
        QJsonArray jsonGroupItems;
        for(auto sci:scg.groupItems){
            QJsonObject jsonItem;
            jsonItem.insert("name",sci.name);
            jsonItem.insert("value",sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems",jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut",jsonGroups);
}
QString Shortcut::toStr(){
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
