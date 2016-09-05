#ifndef TTMCONTENT_H
#define TTMCONTENT_H

#include <QWidget>

class ImageButton;
class TTMContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTMContent(bool fromFileManager, QWidget *parent = 0);

public slots:
    void onImageChanged(const QString &path, bool adaptScreen);
    void updateCollectButton();

signals:
    void resetTransform(bool fitWindow);
    void rotateClockwise();
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);

private:
    ImageButton *m_clBT = nullptr;
    ImageButton *m_adaptImageButton = nullptr;
    ImageButton *m_adaptScreenButtn = nullptr;
    QString m_imageName;
    QString m_imagePath;
};

#endif // TTMCONTENT_H
