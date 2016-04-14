#ifndef SERVICE_POPUP_MENU_MANAGER_H_
#define SERVICE_POPUP_MENU_MANAGER_H_

#include <QObject>
#include <QVariant>
#include <QPoint>

class QAction;
class QMenu;

// Construct and display a popup menu at specific position.
class PopupMenuManager : public QObject {
    Q_OBJECT

public:
    static PopupMenuManager *instance();
    ~PopupMenuManager();

    //    {
    //      "x": 0,
    //      "y": 0,
    //		"items": [
    //			{
    //				"itemId": 10000,
    //				"itemIcon": "",
    //				"itemIconHover": "",
    //				"itemIconInactive": "",
    //				"itemText": "Menu Item Text",
    //              "shortcut": "Ctrl+P",
    //              "isSeparator": false,
    //				"isActive": true,
    //				"checked": false,
    //				"itemSubMenu": {}
    //			},
    //			{
    //				"itemId": 10001,
    //				"itemIcon": "",
    //				"itemIconHover": "",
    //				"itemIconInactive": "",
    //				"itemText": "Menu Item Text",
    //              "shortcut": "Ctrl+P",
    //              "isSeparator": false,
    //				"isActive": true,
    //				"checked": false,
    //				"itemSubMenu": {}
    //			}
    //       ]
    //    }
    void showMenu(const QString &menuJsonContent);
    void hideMenu();

signals:
    void menuHided();
    void menuItemClicked(int menuId);

private:
    explicit PopupMenuManager();
    void initConnections();
    void handleMenuActionTriggered(QAction* action);
    void constructMenu(QMenu* menu, const QJsonArray& content);

private:
    static PopupMenuManager *m_manager;
    QMenu* m_menu;
};


#endif  // SERVICE_POPUP_MENU_MANAGER_H_
