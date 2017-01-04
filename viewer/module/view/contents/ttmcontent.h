#ifndef TTMCONTENT_H
#define TTMCONTENT_H

#include "controller/viewerthememanager.h"
#include <QWidget>

class PushButton;
class QHBoxLayout;
class TTMContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTMContent(bool fromFileManager, QWidget *parent = 0);
public slots:
    void setImage(const QString &path);
    void updateCollectButton();

signals:
    void resetTransform(bool fitWindow);
    void rotateClockwise();
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QHBoxLayout* m_layout;
    PushButton* m_adaptImageBtn;
    PushButton* m_adaptScreenBtn;
    PushButton* m_clBT;
    PushButton* m_rotateLBtn;
    PushButton* m_rotateRBtn;
    PushButton* m_trashBtn;

    QString m_imagePath;
};

#endif // TTMCONTENT_H
