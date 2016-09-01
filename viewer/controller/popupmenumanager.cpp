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

PopupMenuManager::PopupMenuManager(QWidget *parent)
    : QObject(parent),
      m_parentWidget(parent)
{
    m_menu = new QMenu(parent);
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

const QSize PopupMenuManager::sizeHint() const
{
    return m_menu->sizeHint();
}

const QJsonObject PopupMenuManager::createItemObj(const int id,
                                                  const QString &text,
                                                  const bool isSeparator,
                                                  const QString &shortcut,
                                                  const QJsonObject &subMenu)
{
    QJsonObject obj;
    obj["itemId"] = QJsonValue(int(id));
    obj["itemIcon"] = QJsonValue(QString());
    obj["itemIconHover"] = QJsonValue(QString());
    obj["itemIconInactive"] = QJsonValue(QString());
    obj["itemText"] = QJsonValue(text + SHORTCUT_SPLIT_FLAG);
    obj["shortcut"] = QJsonValue(shortcut);
    obj["isSeparator"] = QJsonValue(isSeparator);
    obj["isActive"] = QJsonValue(true);
    obj["checked"] = QJsonValue(false);
    obj["itemSubMenu"] = QJsonValue(subMenu);
    return obj;
}

void PopupMenuManager::setMenuContent(const QString &menuJsonContent)
{
    m_menu->clear();

    // To avoid shortcut overload
    for (QAction *ac : m_parentWidget->actions()) {
       m_parentWidget->removeAction(ac);
    }

    const QJsonObject obj(QJsonDocument::fromJson(menuJsonContent.toUtf8()).
                          object());
    const QJsonArray content(obj["items"].toArray());
    m_pos = QPoint(obj["x"].toInt(), obj["y"].toInt());

    if (content.isEmpty()) {
        qDebug() << "Invalid menu content!";
        return;
    }

    constructMenu(m_menu, content);
}

void PopupMenuManager::handleMenuActionTriggered(QAction* action)
{
    const int menuId = action->data().toInt();
    const QString menuText = action->text();
    emit menuItemClicked(menuId, menuText);
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
            action->setShortcutContext(Qt::WindowShortcut);
            action->setShortcut(QKeySequence(shortcut));
            action->setAutoRepeat(false);
            m_parentWidget->addAction(action);
        }
        else {
            QMenu* subMenu = new QMenu();
            subMenu->setAttribute(Qt::WA_TranslucentBackground);
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

void PopupMenuManager::showMenu()
{
    if (m_pos.isNull()) {
        m_menu->popup(QCursor::pos());
    }
    else {
        m_menu->popup(m_pos);
    }
}

