#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include "databasemanager.h"
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
    void backToMainWindow();
    void gotoSearchPanel(const QString &keyWord = "");
    void gotoPanel(ModulePanel* panel);

    void imageCountChanged(int count);
    void imageInserted(const DatabaseManager::ImageInfo &info);
    void imageRemoved(const QString &name);

    // The following two signal are came from our Product Manager,
    // And Her head has just been kicked by donkey
    void addImageFromTimeline(const QString &targetAlbum);
    void imageAddedToAlbum();

    void editImage(const QString &path);
    void fullScreen(const QString &path);
    void showImageInfo(const QString &path);
    void showInFileManager(const QString &path);
    void startSlideShow(ModulePanel* lastPanel, QStringList paths,
                        QString path = "");

    void viewImage(const ViewInfo &vinfo);

    // Handle by album
    void gotoAlbumPanel(const QString &album = "");
    void createAlbum();
    void importDir(const QString &dir);
    void showProcessTooltip(const QString &message, bool success);
    void insertIntoAlbum(const DatabaseManager::ImageInfo info);
    void removeFromAlbum(const QString &album, const QString &name);

private:
    explicit SignalManager(QObject *parent = 0);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
