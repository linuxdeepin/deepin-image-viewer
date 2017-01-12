#include "application.h"
#include "shortcut.h"

#include "controller/configsetter.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;

    group1.groupName = tr("Image Viewing");
    group2.groupName = tr("Album");
    group3.groupName = tr("Settings");
    group1.groupItems<<
    ShortcutItem(tr("View"), "Enter")<<
    ShortcutItem(tr("Fullscreen"), "F11")<<
    ShortcutItem(tr("Start slideshow"), dApp->setter->value("SHORTCUTVIEW", "Start slideshow").toString())<<
    ShortcutItem(tr("Stop slide show"),  "F5")<<
    ShortcutItem(tr("Copy"), "Ctrl + C")<<
    ShortcutItem(tr("Throw to Trash"),  "Delete")<<
    ShortcutItem(tr("Remove from album"), dApp->setter->value("SHORTCUTVIEW", "Remove from album").toString())<<
    ShortcutItem(tr("Add to my favorite"), dApp->setter->value("SHORTCUTVIEW", "Add to my favorite").toString())<<
    ShortcutItem(tr("Remove from my favorite"), dApp->setter->value("SHORTCUTVIEW", "Remove from my favorite").toString())<<
    ShortcutItem(tr("Rotate clockwise"), dApp->setter->value("SHORTCUTVIEW", "Rotate clockwise").toString())<<
    ShortcutItem(tr("Rotate counterclockwise"), dApp->setter->value("SHORTCUTVIEW", "Rotate counterclockwise").toString())<<
    ShortcutItem(tr("Set as wallpaper"), dApp->setter->value("SHORTCUTVIEW", "Set as wallpaper").toString())<<
    ShortcutItem(tr("Display in file manager"), dApp->setter->value("SHORTCUTVIEW", "Display in file manager").toString())<<
    ShortcutItem(tr("Image info"), dApp->setter->value("SHORTCUTVIEW", "Image info").toString())<<
    ShortcutItem(tr("Previous"), "Left")<<
    ShortcutItem(tr("Next"), "Right")<<
    ShortcutItem(tr("Previous screen"), "PageUp")<<
    ShortcutItem(tr("Next screen"), "PageDown");
    group2.groupItems<<ShortcutItem(tr("New album"), dApp->setter->value("SHORTCUTALBUM", "New album").toString())<<
                       ShortcutItem(tr("Rename"), dApp->setter->value("SHORTCUTALBUM", "Rename").toString());

    group3.groupItems<<ShortcutItem(tr("Help"),  "F1")<<
                       ShortcutItem(tr("Exit"),  "Ctrl + Q")<<
                       ShortcutItem(tr("Display shortcuts"), "Ctrl + Shift + ?");

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
