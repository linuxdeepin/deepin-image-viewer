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
    void hideExtensionPanel();
    void backToMainWindow();
    void gotoSearchPanel(const QString &keyWord = "");
    void gotoPanel(ModulePanel* panel);

    void imageCountChanged(int count);
    void imageInserted(const DatabaseManager::ImageInfo &info);
    void imageRemoved(const QString &name);

    void selectImageFromTimeline(const QString &targetAlbum);

    void editImage(const QString &path);

    void viewImage(const QString &path, const QString &album = "",
                   bool fromFileManager = false);
    void fullScreen(const QString &path);
    void startSlideShow(const QString &path);
    void showImageInfo(const QString &path);
    void showInFileManager(const QString &path);

    // Handle by album
    void gotoAlbumPanel(const QString &album = "");
    void createAlbum();
    void importDir(const QString &dir);
    void showProcessTooltip(const QString &message, bool success);
    void insertIntoAlbum(const QString &album, const QString &name);
    void removeFromAlbum(const QString &album, const QString &name);

private:
    explicit SignalManager(QObject *parent = 0);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
