#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "widgets/blureframe.h"
#include <QJsonObject>
#include <QPointer>

DWIDGET_USE_NAMESPACE

class ImportTip;
class SettingsWindow;
class QHBoxLayout;
class QProcess;
class QMenu;

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

    void updateTipsPos();

private slots:
    void onAbout();
    void onHelp();
    void onNewAlbum();
    void onSetting();
    void onViewShortcut();

private:
    QPointer<QProcess> m_manualPro;
    QHBoxLayout *m_layout;
    QHBoxLayout *m_lLayout;
    QHBoxLayout *m_mLayout;
    QHBoxLayout *m_rLayout;

    ImportTip *m_importTips;
    SettingsWindow *m_settingsWindow;
    QMenu *m_menu;
};

#endif // TOPTOOLBAR_H
