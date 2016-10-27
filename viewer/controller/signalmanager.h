#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include "dbmanager.h"
#include <QObject>

class ModulePanel;
class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();

    // For view images
    struct ViewInfo {
        ModulePanel *lastPanel;                 // For back to the last panel
        bool inDatabase = true;
        bool fullScreen = false;
        QString album = QString();
        QString path;                           // Specific current open one
        QStringList paths = QStringList();      // Limit the view range
    };

signals:
    void enableMainMenu(bool enable);
    void updateTopToolbarLeftContent(QWidget *content);
    void updateTopToolbarMiddleContent(QWidget *content);
    void updateBottomToolbarContent(QWidget *content, bool wideMode = false);
    void updateExtensionPanelContent(QWidget *content);
    void updateExtensionPanelRect();
    void showTopToolbar();
    void hideTopToolbar(bool immediately = false);
    void showBottomToolbar();
    void hideBottomToolbar(bool immediately = false);
    void showExtensionPanel();
    void hideExtensionPanel(bool immediately = false);

    void gotoTimelinePanel();
    void gotoSearchPanel(const QString &keyWord = "");
    void gotoPanel(ModulePanel* panel);
    void backToMainPanel();
    void activeWindow();

    void imagesInserted(const DBImgInfoList infos);
    void imagesRemoved(const QStringList &names);

    void editImage(const QString &path);
    void showImageInfo(const QString &path);
    void showInFileManager(const QString &path);
    void startSlideShow(const ViewInfo &vinfo);

    void viewImage(const ViewInfo &vinfo);

    // Handle by album
    void gotoAlbumPanel(const QString &album = "");
    void createAlbum();
    void importDir(const QString &dir);
    void insertIntoAlbum(const DBImgInfo info);
    void removedFromAlbum(const QString &album, const QStringList &paths);

//    void windowStatesChanged(const Qt::WindowStates state);

private:
    explicit SignalManager(QObject *parent = 0);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
