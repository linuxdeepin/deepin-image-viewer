#ifndef TTMCONTENT_H
#define TTMCONTENT_H

#include "controller/viewerthememanager.h"
#include <QWidget>

class ImageButton;
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
    ImageButton* m_adaptImageBtn;
    ImageButton* m_adaptScreenBtn;
    ImageButton* m_clBT;
    ImageButton* m_rotateLBtn;
    ImageButton* m_rotateRBtn;
    ImageButton* m_trashBtn;

    QString m_imagePath;
};

#endif // TTMCONTENT_H
