#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "widgets/blureframe.h"
#include "aboutwindow.h"
#include <dwindowmaxbutton.h>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QProcess>
#include <QPointer>

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
    bool eventFilter(QObject *obj, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

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
    Dtk::Widget::DWindowMaxButton *m_maxb;
    AboutWindow *m_about;
    PopupMenuManager *m_popupMenu;
};

#endif // TOPTOOLBAR_H
