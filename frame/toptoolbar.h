#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "blureframe.h"
#include "dwindowmaxbutton.h"
#include "dwindowminbutton.h"
#include "dwindowclosebutton.h"
#include "dwindowoptionbutton.h"
#include "dwindowrestorebutton.h"
#include <QHBoxLayout>
#include <QJsonObject>
#include <QProcess>
#include <QPointer>

using namespace Dtk::Widget;

class PopupMenuManager;
class TopToolbar : public BlureFrame
{
    Q_OBJECT
public:
    TopToolbar(QWidget *parent, QWidget *source);
    void setLeftContent(QWidget *content);
    void setMiddleContent(QWidget *content);

signals:
    void moving();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    enum MenuItemId {
        IdCreateAlbum,
        IdImport,
        IdHelp,
        IdAbout,
        IdQuick,
        IdSeparator
    };

    void initWidgets();
    void initMenu();

    QString createMenuContent();

    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId, const QString &text);

    void showManual();

private:
    QPointer<QProcess> m_manualPro;
    QHBoxLayout *m_leftLayout;
    QHBoxLayout *m_middleLayout;
    QWidget *m_leftContent;
    QWidget *m_middleContent;
    QWidget *m_rightContent;
    PopupMenuManager *m_popupMenu;
};

#endif // TOPTOOLBAR_H
