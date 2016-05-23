#ifndef TBCONTENT_H
#define TBCONTENT_H

#include <QWidget>

class DatabaseManager;
class SignalManager;
class ImageButton;
class TBContent : public QWidget
{
    Q_OBJECT
public:
    explicit TBContent(QWidget *parent = 0);
    void updateCollectButton();

public slots:
    void onImageChanged(const QString &name, const QString &path);

signals:
    void showPrevious();
    void toggleSlideShow();
    void showNext();
    void popupDeleteDialog();

private:
    ImageButton *m_clBT;
    SignalManager *m_signalManager;
    DatabaseManager *m_dbManager;
    QString m_imageName;
    QString m_imagePath;
};

#endif // TBCONTENT_H
