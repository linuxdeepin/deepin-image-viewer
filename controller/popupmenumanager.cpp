#include "popupmenumanager.h"
#include "widgets/popupmenustyle.h"

#include <QCursor>
#include <QDebug>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QKeySequence>
#include <QMenu>

PopupMenuManager *PopupMenuManager::m_manager = NULL;
PopupMenuManager *PopupMenuManager::instance()
{
    if (!m_manager) {
        m_manager = new PopupMenuManager();
    }

    return m_manager;
}

PopupMenuManager::PopupMenuManager()
    : QObject()
{
    m_menu = new QMenu();
    m_menu->setAttribute(Qt::WA_TranslucentBackground);
    // Popup frame border-width is 1px
    m_menu->setContentsMargins(1, 1, 1, 1);
    m_menu->setStyle(new PopupMenuStyle());

    initConnections();
}

PopupMenuManager::~PopupMenuManager()
{
    delete m_menu;
}

void PopupMenuManager::handleMenuActionTriggered(QAction* action)
{
    const int menu_id = action->data().toInt();
    emit menuItemClicked(menu_id);
}

// Construct menu or submenu based on its configuration.
void PopupMenuManager::constructMenu(QMenu* menu, const QJsonArray& content)
{
    for (const QJsonValue& item : content) {
        const QJsonObject obj(item.toObject());

        const int itemId = obj["itemId"].toInt();
        const QString itemIcon = obj["itemIcon"].toString();
        const QString itemIconHover = obj["itemIconHover"].toString();
        const QString itemIconInactive = obj["itemIconInactive"].toString();
        const QString itemText = obj["itemText"].toString();
        const QString shortcut = obj["shortcut"].toString();
        const bool isSeparator = obj["isSeparator"].toBool();
        const bool isActive = obj["isActive"].toBool();
        const bool checked = obj["checked"].toBool();
        const QJsonValue subValue = obj["itemSubMenu"];

        if (! isActive) {
            continue;
        }

        QIcon icon;
        icon.addPixmap(QPixmap(itemIcon), QIcon::Normal);
        icon.addPixmap(QPixmap(itemIconHover), QIcon::Selected);
        icon.addPixmap(QPixmap(itemIconInactive), QIcon::Disabled);

        if (isSeparator) {
            menu->addSeparator();
        }
        else if (subValue.toObject().isEmpty()) {
            QAction* action = menu->addAction(icon, itemText);
            action->setCheckable(true);
            action->setChecked(checked);
            action->setData(itemId);
            action->setShortcut(QKeySequence(shortcut));
        }
        else {
            QMenu* subMenu = new QMenu();
            subMenu->setStyle(new PopupMenuStyle());
            QAction* action = menu->addMenu(subMenu);
            action->setIcon(icon);
            action->setText(itemText);

            // Construct submenu.
            constructMenu(subMenu, subValue.toObject()[""].toArray());
        }
    }
}

void PopupMenuManager::initConnections()
{
    connect(m_menu, &QMenu::triggered,
            this, &PopupMenuManager::handleMenuActionTriggered);

    connect(m_menu, &QMenu::aboutToHide, this, &PopupMenuManager::menuHided);
}

void PopupMenuManager::hideMenu()
{
    m_menu->hide();
}

void PopupMenuManager::showMenu(const QString &menuJsonContent)
{
    m_menu->clear();

    const QJsonObject obj(QJsonDocument::fromJson(menuJsonContent.toUtf8()).object());
    const QJsonArray content(obj["items"].toArray());
    const QPoint pos(obj["x"].toInt(), obj["y"].toInt());

    if (content.isEmpty()) {
        qDebug() << "Invalid menu content!";
        return;
    }

    constructMenu(m_menu, content);

    if (pos.isNull()) {
        m_menu->popup(QCursor::pos());
    }
    else {
        m_menu->popup(pos);
    }
}

