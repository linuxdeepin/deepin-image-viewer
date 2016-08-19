#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "widgets/blureframe.h"
#include <dwindowmaxbutton.h>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QProcess>
#include <QPointer>

class AboutWindow;
class PopupMenuManager;
class TopToolbar : public BlureFrame
{
    Q_OBJECT
public:
    TopToolbar(QWidget *parent, QWidget *source);
    void setLeftContent(QWidget *content);
    void setMiddleContent(QWidget *content);

signals:
    void moving(Qt::MouseButton btn);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
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

//    void initAboutWindow();
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
    void showShortCutView();
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
    Qt::MouseButton m_pressBtn;
};

#endif // TOPTOOLBAR_H
