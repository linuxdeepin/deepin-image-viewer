#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>

class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();

signals:
    void updateTopToolbarLeftContent(QWidget *content);
    void updateTopToolbarMiddleContent(QWidget *content);
    void updateBottomToolbarContent(QWidget *content);
    void updateExtensionPanelContent(QWidget *content);
    void showTopToolbar();
    void hideTopToolbar();
    void showBottomToolbar();
    void hideBottomToolbar();
    void showExtensionPanel();
    void hideExtensionPanel();
    void backToMainWindow();

    void imageCountChanged();

    void viewImage(const QString &path);

private:
    explicit SignalManager(QObject *parent = 0);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
