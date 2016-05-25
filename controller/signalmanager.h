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

    void imageCountChanged();
    void imageInserted(const DatabaseManager::ImageInfo &info);

    void selectImageFromTimeline(const QString &targetAlbum);

    void editImage(const QString &path);
    void copyImage(const QString &path);
    void deleteImage(const QString &path);

    void viewImage(const QString &path, const QString &album = "",
                   bool fromFileManager = false);
    void fullScreen(const QString &path);
    void startSlideShow(const QStringList &paths);  // TODO wangbin, NOTE: Data may be repeated

    void gotoAlbumPanel(const QString &album = "");
    void createAlbum();
    void addToAlbum(const QStringList &names);
    void removeFromAlbum(const QString &name, const QString &album);
    void rotate(const QString &path, bool clockwise);
    void updateLabels(const QStringList &labels);
    void showInFileManager(const QString &path);
    void showImageInfo(const QString &path);
    void showProcessTooltip(const QString &message, bool success);

private:
    explicit SignalManager(QObject *parent = 0);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
