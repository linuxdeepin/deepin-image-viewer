#ifndef TTMCONTENT_H
#define TTMCONTENT_H

#include <QWidget>

class DatabaseManager;
class SignalManager;
class ImageButton;
class TTMContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTMContent(bool fromFileManager, QWidget *parent = 0);

public slots:
    void onImageChanged(const QString &name, const QString &path);
    void updateCollectButton();

signals:
    void resetTransform(bool fitWindow);
    void showPrevious();
    void showNext();
    void removed();
    void imageEmpty(bool v);

private:
    DatabaseManager *dbManager() const;

private:
    ImageButton *m_clBT = nullptr;
    SignalManager *m_sManager;
    QString m_imageName;
    QString m_imagePath;
};

#endif // TTMCONTENT_H
