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
class SettingsWindow;
class TopToolbar : public BlurFrame
{
    Q_OBJECT
public:
    TopToolbar(QWidget *parent);
    void setLeftContent(QWidget *content);
    void setMiddleContent(QWidget *content);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
signals:
    void updateImportTipsGeo();
private:
    enum MenuItemId {
        IdCreateAlbum,
        IdSetting,
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
    SettingsWindow *m_settingsWindow;
};

#endif // TOPTOOLBAR_H
