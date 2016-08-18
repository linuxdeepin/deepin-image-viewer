#include "shortcut.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;

    group1.groupName = tr("Image Viewing");
    group1.groupItems<<ShortcutItem(tr("Fullscreen"), "F11 ")<<
                       ShortcutItem(tr("Start slide show"), "F5 ")<<
                       ShortcutItem(tr("Stop slide show"), "F5 ")<<
                       ShortcutItem(tr("Import"), "Ctrl + I ")<<
                       ShortcutItem(tr("Copy"), "Ctrl + C ")<<
                       ShortcutItem(tr("Throw to Trash"), "Delete ")<<
                       ShortcutItem(tr("Remove from album"), "Shift + Delete ")<<
                       ShortcutItem(tr("Add to My favorites"), "Ctrl + K ")<<
                       ShortcutItem(tr("Unfavorites"), "Ctrl + Shift + K ")<<
                       ShortcutItem(tr("Rotate clockwise"), "Ctrl + R ")<<
                       ShortcutItem(tr("Rotate counterclockwise"), "Ctrl + Shift + R ")<<
                       ShortcutItem(tr("Set as wallpaper"), "Ctrl + F8")<<
                       ShortcutItem(tr("Display in file manager"), "Ctrl + D")<<
                       ShortcutItem(tr("Image info"), "Alt + Enter ");
    group2.groupName = tr("Album");
    group2.groupItems<<ShortcutItem(tr("New album"),tr("Ctrl + Shift + N "))<<
                       ShortcutItem(tr("Rename"), "F2");
    group3.groupName = tr("Settings");
    group3.groupItems<<ShortcutItem(tr("Help"),tr("F1 "))<<
                       ShortcutItem(tr("Exit"),tr("Ctrl + Q "));
    m_shortcutGroups << group1 << group2 << group3;

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
