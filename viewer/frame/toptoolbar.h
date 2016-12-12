#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "widgets/blureframe.h"
#include <QJsonObject>
#include <QPointer>

DWIDGET_USE_NAMESPACE

class ImportTip;
class PopupMenuManager;
class SettingsWindow;
class QHBoxLayout;
class QProcess;

class TopToolbar : public BlurFrame
{
    Q_OBJECT
public:
    TopToolbar(QWidget *parent);
    void setLeftContent(QWidget *content);
    void setMiddleContent(QWidget *content);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

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

    void initLeftContent();
    void initMiddleContent();
    void initRightContent();
    void initImportTips();
    void initMenu();
    void initShortcut();
    void initWidgets();

    QString createMenuContent();
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId, const QString &text);

    void showManual();
    void showShortCutView();

    void updateTipsPos();

private:
    QPointer<QProcess> m_manualPro;
    QHBoxLayout *m_layout;
    QHBoxLayout *m_lLayout;
    QHBoxLayout *m_mLayout;
    QHBoxLayout *m_rLayout;

    ImportTip *m_importTips;
    PopupMenuManager *m_popupMenu;
    SettingsWindow *m_settingsWindow;
};

#endif // TOPTOOLBAR_H
