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
    ShortcutGroup group2;
    ShortcutGroup group3;

    group1.groupName = tr("Image Viewing");
    group2.groupName = tr("Album");
    group3.groupName = tr("Settings");
    group1.groupItems<<
    ShortcutItem(tr("View"), "Enter")<<
    ShortcutItem(tr("Fullscreen"), ss(VIEW_GROUP, "Fullscreen"))<<
    ShortcutItem(tr("Slide show"), ss(VIEW_GROUP, "Slide show"))<<
    ShortcutItem(tr("End show"),  ss(VIEW_GROUP, "End show"))<<
    ShortcutItem(tr("Copy"), ss(VIEW_GROUP, "Copy"))<<
    ShortcutItem(tr("Throw to trash"),  "Delete")<<
    ShortcutItem(tr("Remove from album"), ss(VIEW_GROUP, "Remove from album"))<<
    ShortcutItem(tr("Favorite"), ss(VIEW_GROUP, "Favorite"))<<
    ShortcutItem(tr("Unfavorite"), ss(VIEW_GROUP, "Unfavorite"))<<
    ShortcutItem(tr("Rotate clockwise"), ss(VIEW_GROUP, "Rotate clockwise"))<<
    ShortcutItem(tr("Rotate counterclockwise"), ss(VIEW_GROUP, "Rotate counterclockwise"))<<
    ShortcutItem(tr("Set as wallpaper"), ss(VIEW_GROUP, "Set as wallpaper"))<<
    ShortcutItem(tr("Display in file manager"), ss(VIEW_GROUP, "Display in file manager"))<<
    ShortcutItem(tr("Image info"), ss(VIEW_GROUP, "Image info"))<<
    ShortcutItem(tr("Previous"), "Left")<<
    ShortcutItem(tr("Next"), "Right")<<
    ShortcutItem(tr("Previous screen"), "PageUp")<<
    ShortcutItem(tr("Next screen"), "PageDown");
    group2.groupItems<<ShortcutItem(tr("New album"), ss(ALBUM_GROUP, "New album"))<<
                       ShortcutItem(tr("Rename"), ss(ALBUM_GROUP, "Rename"));

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
