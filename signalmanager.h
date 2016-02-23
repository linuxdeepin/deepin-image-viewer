#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>

class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();

signals:
    void updateCenterContent(QWidget *content);
    void updateTopToolbarLeftContent(QWidget *content);
    void updateTopToolbarMiddleContent(QWidget *content);
    void updateBottomToolbarContent(QWidget *content);
    void showTopToolbar();
    void hideTopToolbar();
    void showBottomToolbar();
    void hideBottomToolbar();

private:
    explicit SignalManager(QObject *parent = 0);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
